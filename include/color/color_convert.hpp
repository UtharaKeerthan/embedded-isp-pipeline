#pragma once
#include "types.hpp"

/** @req REQ-COLOR-002 */
IspError rgb_to_yuv_bt709(const uint8_t* rgb, uint8_t* yuv,
                           uint16_t w, uint16_t h);
IspError rgb_to_hsv(const uint8_t* rgb, uint8_t* hsv, uint16_t w, uint16_t h);
