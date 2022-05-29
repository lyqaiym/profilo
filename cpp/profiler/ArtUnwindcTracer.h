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

#include <logger/MultiBufferLogger.h>
#include "profiler/JavaBaseTracer.h"

namespace facebook {
namespace profilo {
namespace profiler {

enum ArtUnwindcVersion {
  kArtUnwindc500,
  kArtUnwindc510,
  kArtUnwindc600,
  kArtUnwindc700,
  kArtUnwindc710,
  kArtUnwindc711,
  kArtUnwindc712,
  kArtUnwindc800,
  kArtUnwindc810,
  kArtUnwindc900
};

template <ArtUnwindcVersion kVersion>
class ArtUnwindcTracer : public JavaBaseTracer {
 public:
  ArtUnwindcTracer();

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

  void startTracing() override;

  void stopTracing() override;
};

#ifdef ANDROID_VERSION_500
template class ArtUnwindcTracer<kArtUnwindc500>;
using ArtUnwindcTracer50 = ArtUnwindcTracer<kArtUnwindc500>;
#endif
#ifdef ANDROID_VERSION_510
template class ArtUnwindcTracer<kArtUnwindc510>;
using ArtUnwindcTracer51 = ArtUnwindcTracer<kArtUnwindc510>;
#endif
#ifdef ANDROID_VERSION_600
template class ArtUnwindcTracer<kArtUnwindc600>;
using ArtUnwindcTracer60 = ArtUnwindcTracer<kArtUnwindc600>;
#endif
#ifdef ANDROID_VERSION_700
template class ArtUnwindcTracer<kArtUnwindc700>;
using ArtUnwindcTracer700 = ArtUnwindcTracer<kArtUnwindc700>;
#endif
#ifdef ANDROID_VERSION_710
template class ArtUnwindcTracer<kArtUnwindc710>;
using ArtUnwindcTracer710 = ArtUnwindcTracer<kArtUnwindc710>;
#endif
#ifdef ANDROID_VERSION_711
template class ArtUnwindcTracer<kArtUnwindc711>;
using ArtUnwindcTracer711 = ArtUnwindcTracer<kArtUnwindc711>;
#endif
#ifdef ANDROID_VERSION_712
template class ArtUnwindcTracer<kArtUnwindc712>;
using ArtUnwindcTracer712 = ArtUnwindcTracer<kArtUnwindc712>;
#endif
#ifdef ANDROID_VERSION_800
template class ArtUnwindcTracer<kArtUnwindc800>;
using ArtUnwindcTracer800 = ArtUnwindcTracer<kArtUnwindc800>;
#endif
#ifdef ANDROID_VERSION_810
template class ArtUnwindcTracer<kArtUnwindc810>;
using ArtUnwindcTracer810 = ArtUnwindcTracer<kArtUnwindc810>;
#endif
#ifdef ANDROID_VERSION_900
template class ArtUnwindcTracer<kArtUnwindc900>;
using ArtUnwindcTracer900 = ArtUnwindcTracer<kArtUnwindc900>;
#endif

} // namespace profiler
} // namespace profilo
} // namespace facebook
