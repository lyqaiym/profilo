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

#include <generated/EntryParser.h>

namespace facebook {
namespace profilo {
namespace writer {

using namespace entries;

class PrintEntryVisitor : public EntryVisitor {
 public:
  PrintEntryVisitor() = delete;
  PrintEntryVisitor(const PrintEntryVisitor&) = delete;

  virtual ~PrintEntryVisitor() = default;

  explicit PrintEntryVisitor(std::ostream& stream);

  virtual void visit(const StandardEntry& data);
  virtual void visit(const FramesEntry& data);
  virtual void visit(const BytesEntry& data);

 private:
  std::ostream& stream_;
};

} // namespace writer
} // namespace profilo
} // namespace facebook
