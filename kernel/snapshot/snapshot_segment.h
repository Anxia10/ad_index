#pragma once
#include "kernel/common/concurrency/thread_pool.h"
#include "kernel/pool/mmap_pool.h"

namespace kernel {
namespace snapshot {

struct SnapshotSegment {
  explicit SnapshotSegment(size_t thread_queue_size) :
    thread_pool(1UL, thread_queue_size),
    fix_store(nullptr),
    var_store(nullptr) {
    thread_pool.Start();
  }
  virtual ~SnapshotSegment() {
    Release();
  }
  virtual int64_t GetSize() {
    return -1L;
  }
  virtual void Release() {
    thread_pool.Stop();
#define DO_RELEASE(obj) do { \
  if (obj != nullptr) { \
    obj->Release(); \
    delete obj; \
    obj = nullptr; \
  } \
} while (0)
    DO_RELEASE(fix_store);
    DO_RELEASE(var_store);
#undef DO_RELEASE
  }

  concurrency::ThreadPool thread_pool;
  pool::MMapPool* fix_store;
  pool::MMapPool* var_store;
};

} // namespace snapshot
} // namespace kernel