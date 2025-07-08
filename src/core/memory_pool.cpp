#include "core/memory_pool.hpp"
#include <cstring>

MemoryPool& MemoryPool::instance() {
    static MemoryPool pool;
    return pool;
}

void* MemoryPool::alloc(size_t bytes) {
    if ((offset_ + bytes) > POOL_SIZE) { return nullptr; }
    void* ptr = &pool_[offset_];
    offset_ += bytes;
    return ptr;
}

void MemoryPool::reset() { offset_ = 0U; }
size_t MemoryPool::used()     const { return offset_; }
size_t MemoryPool::capacity() const { return POOL_SIZE; }
