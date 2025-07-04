#pragma once
#include "types.hpp"

/**
 * @brief LUT-based gamma correction (no floating-point at runtime).
 * @req REQ-GAMMA-001
 * @req REQ-GAMMA-002
 */
IspError gamma_init_lut(float gamma_val);   // call once at startup
IspError gamma_apply(uint8_t* buf, size_t n_pixels);
