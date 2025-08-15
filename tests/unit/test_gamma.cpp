#include <gtest/gtest.h>
#include "core/gamma.hpp"

/**
 * @tc TC-GAMMA-001
 * @covers REQ-GAMMA-001
 * @covers REQ-GAMMA-002
 */
TEST(GammaTest, LutInitAndApply) {
    EXPECT_EQ(gamma_init_lut(2.2F), IspError::OK);
    uint8_t buf[3] = {128U, 64U, 0U};
    EXPECT_EQ(gamma_apply(buf, 1U), IspError::OK);
    // After gamma 2.2, 128/255 -> brighter value
    EXPECT_GT(buf[0], 128U);
    // 0 should remain 0
    EXPECT_EQ(buf[2], 0U);
}

TEST(GammaTest, InvalidGammaReturnsError) {
    EXPECT_EQ(gamma_init_lut(0.5F), IspError::INVALID_PARAM);
    EXPECT_EQ(gamma_init_lut(4.0F), IspError::INVALID_PARAM);
}
