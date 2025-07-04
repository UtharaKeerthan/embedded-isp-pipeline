#pragma once
#include "types.hpp"

struct WbGains { q8_8_t r; q8_8_t g; q8_8_t b; };

/**
 * @brief Gray World white balance gain computation.
 * @req REQ-WB-001
 * @req REQ-WB-002
 */
WbGains  wb_compute_gray_world(const uint8_t* rgb, uint16_t w, uint16_t h);
IspError wb_apply(uint8_t* rgb, uint16_t w, uint16_t h, WbGains gains);
