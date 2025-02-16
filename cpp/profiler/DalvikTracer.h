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

#include <unistd.h>

#include <dalvik/internals.h>

#include <logger/MultiBufferLogger.h>
#include <profiler/JavaBaseTracer.h>

namespace facebook {
namespace profilo {
namespace profiler {

using dvmThreadSelf_t = Thread* (*)();

class DalvikTracer : public JavaBaseTracer {
 public:
  DalvikTracer();

  DalvikTracer(const DalvikTracer& obj) = delete;
  DalvikTracer& operator=(DalvikTracer obj) = delete;

  StackCollectionRetcode collectStack(
      ucontext_t* ucontext,
      int64_t* frames,
      uint16_t& depth,
      uint16_t max_depth) override;

  StackCollectionRetcode collectJavaStack(
      ucontext_t* ucontext,
      int64_t* frames,
      char const** method_names,
      char const** class_descriptors,
      uint16_t& depth,
      uint16_t max_depth) override;

  void flushStack(
      MultiBufferLogger& logger,
      int64_t* frames,
      uint16_t depth,
      int tid,
      int64_t time_) override;

  void prepare() override;

  void stopTracing() override;

  void startTracing() override;

 private:
  dvmThreadSelf_t dvmThreadSelf_;
};

} // namespace profiler
} // namespace profilo
} // namespace facebook
