#include <gtest/gtest.h>
#include "core/pipeline.hpp"
#include "core/memory_pool.hpp"
#include <cstring>
#include <chrono>

/**
 * @tc TC-INT-001
 * @covers REQ-ISP-001
 * @covers REQ-ISP-004
 */
TEST(PipelineIntegration, SmallFrameRunsWithinBudget) {
    static uint16_t bayer[64*64];
    static uint8_t  yuv  [64*64*3];
    memset(bayer, 0, sizeof(bayer));

    IspPipeline p;
    ASSERT_EQ(p.init(2.2F), IspError::OK);

    MemoryPool::instance().reset();
    auto t0  = std::chrono::steady_clock::now();
    IspError err = p.run(bayer, yuv, 64U, 64U);
    auto t1  = std::chrono::steady_clock::now();

    EXPECT_EQ(err, IspError::OK);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
    // 64x64 frame should be orders of magnitude under the 33ms budget
    EXPECT_LT(ms, 33L);
}
