#pragma once
#include "types.hpp"

/**
 * @brief Top-level ISP pipeline orchestrator.
 * @req REQ-ISP-001
 * @req REQ-ISP-004
 */
class IspPipeline {
public:
    IspError init(float gamma_val = 2.2F);
    IspError run(const uint16_t* bayer_in, uint8_t* yuv_out,
                 uint16_t width, uint16_t height);
};
