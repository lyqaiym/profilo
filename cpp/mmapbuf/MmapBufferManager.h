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

#include <xplat_init/xplat_init.h>
#include <fbjni/fbjni.h>
#include <jni.h>
#include <pthread.h>

#include <linker/locks.h>
#include <mmapbuf/Buffer.h>
#include <mmapbuf/JBuffer.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace fbjni = facebook::jni;

namespace facebook {
namespace profilo {
namespace mmapbuf {

class MmapBufferManager : public fbjni::HybridClass<MmapBufferManager> {
 public:
  constexpr static auto kJavaDescriptor =
      "Lcom/facebook/profilo/mmapbuf/core/MmapBufferManager;";

  static fbjni::local_ref<MmapBufferManager::jhybriddata> initHybrid(
      fbjni::alias_ref<jclass>);
  //
  // Allocates TraceBuffer according to the passed parameters in a file.
  // Returns a non-null reference if successful, nullptr if not.
  //
  std::shared_ptr<Buffer> allocateBufferAnonymous(int32_t buffer_slots_size);

  fbjni::local_ref<JBuffer::javaobject> allocateBufferAnonymousForJava(
      int32_t buffer_slots_size);

  //
  // Allocates TraceBuffer according to the passed parameters in a file.
  // Returns a non-null reference if successful, nullptr if not.
  //
  std::shared_ptr<Buffer> allocateBufferFile(
      int32_t buffer_slots_size,
      const std::string& path);

  fbjni::local_ref<JBuffer::javaobject> allocateBufferFileForJava(
      int32_t buffer_slots_size,
      const std::string& path);

  bool deallocateBufferForJava(JBuffer* buffer);
  bool deallocateBuffer(std::shared_ptr<Buffer> buffer);

  void forEachBuffer(std::function<void(Buffer&)> fn);

  static void registerNatives();

 private:
  void registerBuffer(std::shared_ptr<Buffer> buffer);

  pthread_rwlock_t buffers_lock_ = PTHREAD_RWLOCK_INITIALIZER;
  std::vector<std::shared_ptr<Buffer>> buffers_;

  friend class MmapBufferManagerTestAccessor;
};

} // namespace mmapbuf
} // namespace profilo
} // namespace facebook
