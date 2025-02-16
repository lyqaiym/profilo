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

#include <api/ExternalApi.h>
#include <logger/MultiBufferLogger.h>
#include <ucontext.h>
#include <unistd.h>

static constexpr const uint32_t kSystemDexId = 0xFFFFFFFF;

namespace facebook {
namespace profilo {

using logger::MultiBufferLogger;

namespace profiler {

namespace tracers {
enum Tracer : uint32_t {
  DALVIK = 1,
  NATIVE = 1 << 2,

  // Keep these ART_UNWINDC_* values in sync with
  // JavaBaseTracer::isJavaTracer()
  ART_UNWINDC_6_0 = 1 << 4,
  ART_UNWINDC_7_0_0 = 1 << 5,
  ART_UNWINDC_7_1_0 = 1 << 6,
  ART_UNWINDC_7_1_1 = 1 << 7,
  ART_UNWINDC_7_1_2 = 1 << 8,
  JAVASCRIPT = 1 << 9,
  ART_UNWINDC_5_0 = 1 << 10,
  ART_UNWINDC_5_1 = 1 << 11,
  ART_UNWINDC_8_0_0 = 1 << 12,
  ART_UNWINDC_8_1_0 = 1 << 13,
  ART_UNWINDC_9_0_0 = 1 << 14,
};
}

class BaseTracer {
 public:
  virtual ~BaseTracer() = default;

  virtual StackCollectionRetcode collectStack(
      ucontext_t* ucontext,
      int64_t* frames,
      uint16_t& depth,
      uint16_t max_depth) = 0;

  virtual void flushStack(
      MultiBufferLogger& logger,
      int64_t* frames,
      uint16_t depth,
      int tid,
      int64_t time_) = 0;

  virtual void startTracing() = 0;

  virtual void stopTracing() = 0;

  // May be called to initialize static state. Must be always safe.
  virtual void prepare() = 0;
};

} // namespace profiler
} // namespace profilo
} // namespace facebook
