#pragma once
#include "types.hpp"
/** @req REQ-HIST-001 */
IspError compute_histogram(const uint8_t* channel, uint16_t w, uint16_t h,
                            uint32_t* hist_out);
