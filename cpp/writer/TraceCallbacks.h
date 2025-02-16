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

#include <writer/AbortReason.h>

namespace facebook {
namespace profilo {
namespace writer {

struct TraceCallbacks {
  virtual ~TraceCallbacks() {}

  virtual void onTraceStart(int64_t trace_id, int32_t flags) = 0;

  virtual void onTraceEnd(int64_t trace_id) = 0;

  virtual void onTraceAbort(int64_t trace_id, AbortReason reason) = 0;
};

} // namespace writer
} // namespace profilo
} // namespace facebook
