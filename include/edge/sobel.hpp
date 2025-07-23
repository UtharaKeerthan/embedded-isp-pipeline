#pragma once
#include "types.hpp"

/**
 * @brief Sobel gradient magnitude and direction.
 * @req REQ-EDGE-001
 */
IspError sobel(const uint8_t* gray, uint8_t* magnitude,
               uint16_t* direction, uint16_t w, uint16_t h);
