#pragma once
#include "types.hpp"

/**
 * @brief 3x3 Color Correction Matrix in Q4.12 fixed-point.
 * @req REQ-COLOR-001
 */
struct Ccm3x3 { q4_12_t coef[3][3]; };
IspError ccm_apply(uint8_t* rgb, uint16_t w, uint16_t h, const Ccm3x3& m);
