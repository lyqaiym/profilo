/**
 * Copyright 2004-present, Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <errno.h>
#include <deque>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <generated/Entry.h>
#include <generated/EntryParser.h>
#include <writer/AbortReason.h>
#include <writer/ScopedThreadPriority.h>
#include <writer/TraceCallbacks.h>
#include <writer/TraceFileHelpers.h>

#include <zstr/src/zstr.hpp>

namespace facebook {
namespace profilo {
namespace writer {

using namespace facebook::profilo::entries;

class TraceLifecycleVisitor : public EntryVisitor {
 public:
  TraceLifecycleVisitor(
      const std::string& trace_folder,
      const std::string& trace_prefix,
      std::shared_ptr<TraceCallbacks> callbacks,
      const std::vector<std::pair<std::string, std::string>>& headers,
      int64_t trace_id,
      std::function<void(TraceLifecycleVisitor& visitor)>
          trace_backward_callback = nullptr);

  virtual void visit(const StandardEntry& entry) override;
  virtual void visit(const FramesEntry& entry) override;
  virtual void visit(const BytesEntry& entry) override;

  void abort(AbortReason reason);

  inline bool done() const {
    return done_;
  }

  inline int64_t getTraceID() const {
    return expected_trace_;
  }

 private:
  const std::string trace_folder_;
  const std::string trace_prefix_;
  const std::vector<std::pair<std::string, std::string>> trace_headers_;
  std::unique_ptr<std::ofstream> output_;

  // chain of delegates
  std::deque<std::unique_ptr<EntryVisitor>> delegates_;
  int64_t expected_trace_;
  std::shared_ptr<TraceCallbacks> callbacks_;
  bool started_;
  bool done_;
  std::unique_ptr<ScopedThreadPriority> thread_priority_;
  std::function<void(TraceLifecycleVisitor& visitor)> trace_backward_callback_;

  inline bool hasDelegate() {
    return !delegates_.empty();
  }

  void onTraceStart(int64_t trace_id, int32_t flags);
  void onTraceAbort(int64_t trace_id, AbortReason reason);
  void onTraceEnd(int64_t trace_id);
  void cleanupState();
};

} // namespace writer
} // namespace profilo
} // namespace facebook
