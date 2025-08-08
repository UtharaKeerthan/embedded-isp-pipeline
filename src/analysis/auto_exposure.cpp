#include "analysis/auto_exposure.hpp"
static constexpr uint8_t  AE_TARGET  = 128U;
static constexpr float    AE_GAIN_K  = 0.1F;
IspError ae_update(AeState& s, const uint8_t* gray, uint16_t w, uint16_t h) {
    if (!gray) return IspError::NULL_BUFFER;
    uint64_t sum = 0;
    const size_t n = (size_t)w*h;
    for (size_t i = 0; i < n; i++) sum += gray[i];
    float mean = (float)sum / (float)n;
    float err  = (float)AE_TARGET - mean;
    float new_exp = (float)s.exposure_us * (1.0F + AE_GAIN_K * err / 255.0F);
    s.exposure_us = (uint32_t)((new_exp < 100.0F) ? 100.0F : new_exp);
    return IspError::OK;
}
