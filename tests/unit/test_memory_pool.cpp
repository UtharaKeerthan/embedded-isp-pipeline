#include <gtest/gtest.h>
#include "core/memory_pool.hpp"

/**
 * @tc TC-MEM-001
 * @covers REQ-ISP-002
 * @covers REQ-MEM-001
 */
TEST(MemoryPoolTest, AllocAndReset) {
    MemoryPool& pool = MemoryPool::instance();
    pool.reset();
    EXPECT_EQ(pool.used(), 0U);
    void* p1 = pool.alloc(1024U);
    EXPECT_NE(p1, nullptr);
    EXPECT_EQ(pool.used(), 1024U);
    pool.reset();
    EXPECT_EQ(pool.used(), 0U);
}

TEST(MemoryPoolTest, ExhaustionReturnsNull) {
    MemoryPool& pool = MemoryPool::instance();
    pool.reset();
    void* p = pool.alloc(pool.capacity() + 1U);
    EXPECT_EQ(p, nullptr);
}
