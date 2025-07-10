#pragma once
#include "types.hpp"

/** @req REQ-HIST-001  @req REQ-HIST-002 */
struct ClaheConfig { uint16_t tile_w = 64U; uint16_t tile_h = 64U; uint8_t clip = 40U; };
IspError hist_equalize(uint8_t* gray, uint16_t w, uint16_t h);
IspError clahe(uint8_t* gray, uint16_t w, uint16_t h, ClaheConfig cfg);
