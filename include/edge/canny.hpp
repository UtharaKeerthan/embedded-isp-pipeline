#pragma once
#include "types.hpp"

/**
 * @brief Canny multi-stage edge detector.
 * @req REQ-EDGE-002
 * @req REQ-EDGE-003
 */
struct CannyConfig {
    uint8_t  gaussian_sigma = 1U;
    uint8_t  t_high         = 100U;
    uint8_t  t_low          = 40U;
};
IspError canny(const uint8_t* gray, uint8_t* edges,
               uint16_t w, uint16_t h, CannyConfig cfg);
