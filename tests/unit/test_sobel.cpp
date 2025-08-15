#include <gtest/gtest.h>
#include "edge/sobel.hpp"

/**
 * @tc TC-EDGE-001
 * @covers REQ-EDGE-001
 */
TEST(SobelTest, DetectsVerticalEdge) {
    // 5x5 image: left half dark, right half bright
    static uint8_t gray[5*5] = {
        0,0,255,255,255,
        0,0,255,255,255,
        0,0,255,255,255,
        0,0,255,255,255,
        0,0,255,255,255
    };
    static uint8_t  mag[5*5] = {0};
    IspError err = sobel(gray, mag, nullptr, 5U, 5U);
    EXPECT_EQ(err, IspError::OK);
    // Magnitude should be high at the edge column
    EXPECT_GT(mag[1*5+2], 0U);
}
