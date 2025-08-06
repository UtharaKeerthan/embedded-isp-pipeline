#pragma once
#include "types.hpp"
/** @req REQ-DPC-001 */
IspError dead_pixel_correct(uint16_t* bayer, uint16_t w, uint16_t h,
                             const uint32_t* defect_map, uint32_t defect_count);
