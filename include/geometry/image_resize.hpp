#pragma once
#include "types.hpp"
/** @req REQ-RESIZE-001 */
IspError bilinear_resize(const uint8_t* src, uint16_t src_w, uint16_t src_h,
                          uint8_t* dst, uint16_t dst_w, uint16_t dst_h,
                          uint8_t channels);
