#pragma once
#include "types.hpp"

/**
 * @brief Static memory pool - all pipeline buffers allocated here.
 * @details No malloc/new anywhere in the codebase. Pool reset per frame.
 * @req REQ-ISP-002
 * @req REQ-MEM-001
 */
class MemoryPool {
public:
    static MemoryPool& instance();
    void*  alloc(size_t bytes);
    void   reset();           // Call at frame start
    size_t used()     const;
    size_t capacity() const;
private:
    MemoryPool() = default;
    static constexpr size_t POOL_SIZE = FRAME_BUF_SIZE * 8U;
    uint8_t pool_[POOL_SIZE];
    size_t  offset_ = 0U;
};
