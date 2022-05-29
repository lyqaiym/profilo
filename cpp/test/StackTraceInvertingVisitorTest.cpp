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

#include <limits>
#include <sstream>

#include <gtest/gtest.h>

#include <generated/EntryParser.h>
#include <writer/PrintEntryVisitor.h>
#include <writer/StackTraceInvertingVisitor.h>

using namespace facebook::profilo::entries;
using namespace facebook::profilo::writer;

namespace facebook {
namespace profilo {

TEST(StackInvertingVisitorTest, testStacksAreInverted) {
  std::stringstream stream;
  PrintEntryVisitor print(stream);
  StackTraceInvertingVisitor stack(print);

  int64_t frames[] = {300, 200, 100};
  stack.visit(FramesEntry{
      .id = 1,
      .type = EntryType::STACK_FRAME,
      .timestamp = 1,
      .tid = 1,
      .matchid = 10,
      .frames =
          {
              .values = frames,
              .size = 3,
          },
  });

  int64_t otherframes[] = {3000, 2000, 1000};
  stack.visit(FramesEntry{
      .id = 2,
      .type = EntryType::STACK_FRAME,
      .timestamp = 2,
      .tid = 2,
      .matchid = 10,
      .frames =
          {
              .values = otherframes,
              .size = 3,
          },
  });

  EXPECT_EQ(
      stream.str(),
      "1|STACK_FRAME|1|1|0|10|100\n"
      "1|STACK_FRAME|1|1|0|10|200\n"
      "1|STACK_FRAME|1|1|0|10|300\n"
      "2|STACK_FRAME|2|2|0|10|1000\n"
      "2|STACK_FRAME|2|2|0|10|2000\n"
      "2|STACK_FRAME|2|2|0|10|3000\n");
}

} // namespace profilo
} // namespace facebook
