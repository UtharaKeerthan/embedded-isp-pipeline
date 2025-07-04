#pragma once
#include "types.hpp"

/**
 * @brief Converts raw Bayer RGGB buffer to interleaved RGB.
 * @details Bilinear interpolation, fixed-point only, no dynamic allocation.
 * @req REQ-ISP-001
 * @req REQ-ISP-002
 * @req REQ-ISP-003
 * @param bayer  Input: Bayer uint16_t row-major buffer (RGGB pattern)
 * @param rgb    Output: uint8_t RGB buffer, caller-allocated via MemoryPool
 * @param width  Frame width in pixels
 * @param height Frame height in pixels
 */
IspError demosaic_rggb(const uint16_t* bayer, uint8_t* rgb,
                        uint16_t width, uint16_t height);
