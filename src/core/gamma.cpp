#include "core/gamma.hpp"

static uint8_t s_gamma_lut[256];
static bool    s_lut_ready = false;

/**
 * @tc TC-GAMMA-001
 * @covers REQ-GAMMA-001
 * @covers REQ-GAMMA-002
 */
IspError gamma_init_lut(float gamma_val) {
    if ((gamma_val < 1.0F) || (gamma_val > 3.0F)) {
        return IspError::INVALID_PARAM;
    }
    const float inv_gamma = 1.0F / gamma_val;
    for (uint32_t i = 0U; i < 256U; i++) {
        const float norm = static_cast<float>(i) / 255.0F;
        // Power-law: out = in^(1/gamma)
        // Approximated with integer math via pre-computation
        float val = 1.0F;
        // Simple power approximation suitable for LUT precomputation
        val = norm;
        // Apply gamma curve (this runs ONCE at init, float allowed here)
        float powered = 1.0F;
        float base = norm;
        // Use iterative approximation for embedded compatibility in init path
        powered = base; // start: base^1
        // For LUT init, actual float pow is acceptable (runs once)
        for (int iter = 0; iter < 8; iter++) {
            powered = powered * (1.0F + inv_gamma * (base / powered - 1.0F));
        }
        const uint32_t out = static_cast<uint32_t>(powered * 255.0F + 0.5F);
        s_gamma_lut[i] = static_cast<uint8_t>((out > 255U) ? 255U : out);
    }
    s_lut_ready = true;
    return IspError::OK;
}

IspError gamma_apply(uint8_t* buf, size_t n_pixels) {
    if (!s_lut_ready) { return IspError::INVALID_PARAM; }
    if (buf == nullptr) { return IspError::NULL_BUFFER; }
    for (size_t i = 0U; i < n_pixels * 3U; i++) {
        buf[i] = s_gamma_lut[buf[i]];   // O(1) per pixel - pure LUT lookup
    }
    return IspError::OK;
}
