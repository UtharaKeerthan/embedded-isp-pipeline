#include <gtest/gtest.h>
#include "core/demosaic.hpp"
#include "core/memory_pool.hpp"
#include <cstring>

/**
 * @tc TC-ISP-001
 * @covers REQ-ISP-001
 */
TEST(DemosaicTest, BasicRGGBConversion) {
    static uint16_t bayer[4*4];
    static uint8_t  rgb  [4*4*3];
    memset(bayer, 0, sizeof(bayer));
    // Set a known red pixel at (0,0)
    bayer[0] = 1023U;
    IspError err = demosaic_rggb(bayer, rgb, 4U, 4U);
    EXPECT_EQ(err, IspError::OK);
}

/**
 * @tc TC-ISP-002
 * @covers REQ-ISP-002
 */
TEST(DemosaicTest, NullInputReturnsError) {
    static uint8_t rgb[4*4*3];
    EXPECT_EQ(demosaic_rggb(nullptr, rgb, 4U, 4U), IspError::NULL_BUFFER);
    EXPECT_EQ(demosaic_rggb(nullptr, nullptr, 4U, 4U), IspError::NULL_BUFFER);
}
