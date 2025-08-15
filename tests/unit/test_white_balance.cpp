#include <gtest/gtest.h>
#include "core/white_balance.hpp"
#include <cstring>

/**
 * @tc TC-WB-001
 * @covers REQ-WB-001
 */
TEST(WhiteBalanceTest, GainApplicationClampsAt255) {
    static uint8_t rgb[3] = {200, 100, 200};
    WbGains g; g.r = 512; g.g = 256; g.b = 256; // r gain = 2.0
    IspError err = wb_apply(rgb, 1U, 1U, g);
    EXPECT_EQ(err, IspError::OK);
    EXPECT_EQ(rgb[0], 255U); // clamped
}

/**
 * @tc TC-WB-002
 * @covers REQ-WB-002
 */
TEST(WhiteBalanceTest, GrayWorldProducesEqualMeans) {
    static uint8_t rgb[3*3*3];
    // Fill with equal R,G,B -> gains should all be 1.0
    for (int i = 0; i < 9; i++) { rgb[i*3]=128; rgb[i*3+1]=128; rgb[i*3+2]=128; }
    WbGains g = wb_compute_gray_world(rgb, 3U, 3U);
    EXPECT_NEAR(q8_8_to_float(g.r), 1.0F, 0.02F);
}
