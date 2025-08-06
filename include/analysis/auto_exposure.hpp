#pragma once
#include "types.hpp"
/** @req REQ-AE-001 */
struct AeState { uint32_t exposure_us; uint8_t gain; };
IspError ae_update(AeState& state, const uint8_t* gray, uint16_t w, uint16_t h);
