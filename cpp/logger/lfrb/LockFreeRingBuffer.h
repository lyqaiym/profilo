/*
 * Copyright 2017 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <unistd.h>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <type_traits>

#include <logger/lfrb/TurnSequencer.h>

namespace facebook {
namespace profilo {
namespace mmapbuf {
struct Buffer;
} // namespace mmapbuf

namespace logger {
namespace lfrb {

namespace {
//
// Constructs n objects of type typename iterator_traits<ForwardIt>::value_type
// in the uninitialized storage starting at first. As
// std::uninitialized_default_construct_n symbol is not yet available for our
// Android builds using this analogous implementation.
//
template <class ForwardIt, class Size>
ForwardIt _uninitialized_default_construct_n(ForwardIt first, Size n) {
  using T = typename std::iterator_traits<ForwardIt>::value_type;
  ForwardIt current = first;
  for (; n > 0; (void)++current, --n) {
    new (static_cast<void*>(std::addressof(*current))) T;
  }
  return current;
}

//
// Destruction of flexible array member
//
template <class ForwardIt, class Size>
ForwardIt _destroy_n(ForwardIt first, Size n) {
  using T = typename std::iterator_traits<ForwardIt>::value_type;
  for (; n > 0; (void)++first, --n)
    std::addressof(*first)->~T();
  return first;
}
} // namespace

namespace detail {
template <typename T, template <typename> class Atom = std::atomic>
class RingBufferSlot;
} // namespace detail

/// LockFreeRingBuffer<T> is a fixed-size, concurrent ring buffer with the
/// following semantics:
///
///  1. Writers cannot block on other writers UNLESS they are <capacity> writes
///     apart from each other (writing to the same slot after a wrap-around)
///  2. Writers cannot block on readers
///  3. Readers can wait for writes that haven't occurred yet
///  4. Readers can detect if they are lagging behind
///
/// In this sense, reads from this buffer are best-effort but writes
/// are guaranteed.
///
/// Another way to think about this is as an unbounded stream of writes. The
/// buffer contains the last <capacity> writes but readers can attempt to read
/// any part of the stream, even outside this window. The read API takes a
/// Cursor that can point anywhere in this stream of writes. Reads from the
/// "future" can optionally block but reads from the "past" will always fail.
///

template <typename T, template <typename> class Atom = std::atomic>
class LockFreeRingBuffer {
  static_assert(
      std::is_nothrow_default_constructible<T>::value,
      "Element type must be nothrow default constructible");

  static_assert(
      __has_trivial_copy(T),
      "Element type must be trivially copyable");

 public:
  static constexpr size_t calculateAllocationSize(size_t entryCount) {
    return sizeof(LockFreeRingBuffer<T, Atom>) +
        entryCount * sizeof(detail::RingBufferSlot<T, Atom>);
  }

  /// Opaque pointer to a past or future write.
  /// Can be moved relative to its current location but not in absolute terms.
  struct Cursor {
    explicit Cursor(uint64_t initialTicket) noexcept : ticket(initialTicket) {}

    /// Returns true if this cursor now points to a different
    /// write, false otherwise.
    bool moveForward(uint64_t steps = 1) noexcept {
      uint64_t prevTicket = ticket;
      ticket += steps;
      return prevTicket != ticket;
    }

    /// Returns true if this cursor now points to a previous
    /// write, false otherwise.
    bool moveBackward(uint64_t steps = 1) noexcept {
      uint64_t prevTicket = ticket;
      if (steps > ticket) {
        ticket = 0;
      } else {
        ticket -= steps;
      }
      return prevTicket != ticket;
    }

   protected: // for test visibility reasons
    uint64_t ticket;
    friend class LockFreeRingBuffer;
  };

  LockFreeRingBuffer() = delete;
  LockFreeRingBuffer(LockFreeRingBuffer const&) = delete;
  LockFreeRingBuffer& operator=(LockFreeRingBuffer const&) = delete;

  uint32_t capacity() {
    return capacity_;
  }

  /// Perform a single write of an object of type T.
  /// Writes can block iff a previous writer has not yet completed a write
  /// for the same slot (before the most recent wrap-around).
  void write(T& value) noexcept {
    uint64_t ticket = ticket_.fetch_add(1);
    slots_[idx(ticket)].write(turn(ticket), value);
  }

  /// Perform a single write of an object of type T.
  /// Writes can block iff a previous writer has not yet completed a write
  /// for the same slot (before the most recent wrap-around).
  /// Returns a Cursor pointing to the just-written T.
  Cursor writeAndGetCursor(T& value) noexcept {
    uint64_t ticket = ticket_.fetch_add(1);
    slots_[idx(ticket)].write(turn(ticket), value);
    return Cursor(ticket);
  }

  /// Read the value at the cursor.
  /// Returns true if the read succeeded, false otherwise. If the return
  /// value is false, dest is to be considered partially read and in an
  /// inconsistent state. Readers are advised to discard it.
  bool tryRead(T& dest, const Cursor& cursor) noexcept {
    return slots_[idx(cursor.ticket)].tryRead(dest, turn(cursor.ticket));
  }

  /// Read the value at the cursor or block if the write has not occurred yet.
  /// Returns true if the read succeeded, false otherwise. If the return
  /// value is false, dest is to be considered partially read and in an
  /// inconsistent state. Readers are advised to discard it.
  bool waitAndTryRead(T& dest, const Cursor& cursor) noexcept {
    return slots_[idx(cursor.ticket)].waitAndTryRead(dest, turn(cursor.ticket));
  }

  /// Returns a Cursor pointing to the first write that has not occurred yet.
  Cursor currentHead() noexcept {
    return Cursor(ticket_.load());
  }

  /// Returns a Cursor pointing to a currently readable write.
  /// skipFraction is a value in the [0, 1] range indicating how far into the
  /// currently readable window to place the cursor. 0 means the
  /// earliest readable write, 1 means the latest readable write (if any).
  Cursor currentTail(double skipFraction = 0.0) noexcept {
    assert(skipFraction >= 0.0 && skipFraction <= 1.0);
    uint64_t ticket = ticket_.load();

    uint64_t backStep = llround((1.0 - skipFraction) * capacity_);

    // always try to move at least one step backward to something readable
    backStep = std::max<uint64_t>(1, backStep);

    // can't go back more steps than we've taken
    backStep = std::min(ticket, backStep);

    return Cursor(ticket - backStep);
  }

 private:
  const uint32_t capacity_;
  Atom<uint64_t> ticket_;
  detail::RingBufferSlot<T, Atom> slots_[];

  static LockFreeRingBuffer<T, Atom>* allocateAt(uint32_t capacity, void* ptr) {
    LockFreeRingBuffer<T, Atom>* buffer =
        new (ptr) LockFreeRingBuffer<T, Atom>(capacity);
    _uninitialized_default_construct_n(buffer->slots_, capacity);

    return buffer;
  }

  explicit LockFreeRingBuffer(uint32_t capacity) noexcept
      : capacity_(capacity), ticket_(0) {}

  ~LockFreeRingBuffer() {
    _destroy_n(slots_, capacity_);
  }

  uint32_t idx(uint64_t ticket) noexcept {
    return ticket % capacity_;
  }

  uint32_t turn(uint64_t ticket) noexcept {
    return (uint32_t)(ticket / capacity_);
  }

  friend struct facebook::profilo::mmapbuf::Buffer;
  friend class LockFreeRingBufferTestAccessor;
}; // LockFreeRingBuffer

namespace detail {
template <typename T, template <typename> class Atom>
class RingBufferSlot {
 public:
  explicit RingBufferSlot() noexcept : sequencer_(), data() {}

  void write(const uint32_t turn, T& value) noexcept {
    Atom<uint32_t> cutoff(0);
    sequencer_.waitForTurn(turn * 2, cutoff, false);

    // Change to an odd-numbered turn to indicate write in process
    sequencer_.completeTurn(turn * 2);

    data = std::move(value);
    sequencer_.completeTurn(turn * 2 + 1);
    // At (turn + 1) * 2
  }

  bool waitAndTryRead(T& dest, uint32_t turn) noexcept {
    uint32_t desired_turn = (turn + 1) * 2;
    Atom<uint32_t> cutoff(0);
    if (sequencer_.tryWaitForTurn(desired_turn, cutoff, false) !=
        TurnSequencer<Atom>::TryWaitResult::SUCCESS) {
      return false;
    }
    memcpy(&dest, &data, sizeof(T));

    // if it's still the same turn, we read the value successfully
    return sequencer_.isTurn(desired_turn);
  }

  bool tryRead(T& dest, uint32_t turn) noexcept {
    // The write that started at turn 0 ended at turn 2
    if (!sequencer_.isTurn((turn + 1) * 2)) {
      return false;
    }
    memcpy(&dest, &data, sizeof(T));

    // if it's still the same turn, we read the value successfully
    return sequencer_.isTurn((turn + 1) * 2);
  }

 private:
  TurnSequencer<Atom> sequencer_;
  T data;
  friend class LockFreeRingBuffer<T>;
}; // RingBufferSlot

} // namespace detail

} // namespace lfrb

} // namespace logger

} // namespace profilo

} // namespace facebook
