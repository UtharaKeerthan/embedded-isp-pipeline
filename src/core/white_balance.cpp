#include "core/white_balance.hpp"

/**
 * @tc TC-WB-001
 * @covers REQ-WB-002
 */
WbGains wb_compute_gray_world(const uint8_t* rgb, uint16_t w, uint16_t h) {
    uint64_t sum_r = 0U, sum_g = 0U, sum_b = 0U;
    const size_t n = static_cast<size_t>(w) * h;
    for (size_t i = 0U; i < n; i++) {
        sum_r += rgb[i * 3U];
        sum_g += rgb[i * 3U + 1U];
        sum_b += rgb[i * 3U + 2U];
    }
    const float mean_r = static_cast<float>(sum_r) / static_cast<float>(n);
    const float mean_g = static_cast<float>(sum_g) / static_cast<float>(n);
    const float mean_b = static_cast<float>(sum_b) / static_cast<float>(n);
    WbGains g;
    g.r = float_to_q8_8(mean_g / (mean_r + 0.001F));
    g.g = float_to_q8_8(1.0F);
    g.b = float_to_q8_8(mean_g / (mean_b + 0.001F));
    return g;
}

/**
 * @tc TC-WB-002
 * @covers REQ-WB-001
 */
IspError wb_apply(uint8_t* rgb, uint16_t w, uint16_t h, WbGains gains) {
    if (rgb == nullptr) { return IspError::NULL_BUFFER; }
    const size_t n = static_cast<size_t>(w) * h;
    for (size_t i = 0U; i < n; i++) {
        int32_t r = (static_cast<int32_t>(rgb[i*3U])   * gains.r) >> 8;
        int32_t g = (static_cast<int32_t>(rgb[i*3U+1U]) * gains.g) >> 8;
        int32_t b = (static_cast<int32_t>(rgb[i*3U+2U]) * gains.b) >> 8;
        rgb[i*3U]   = static_cast<uint8_t>((r > 255) ? 255 : r);
        rgb[i*3U+1U] = static_cast<uint8_t>((g > 255) ? 255 : g);
        rgb[i*3U+2U] = static_cast<uint8_t>((b > 255) ? 255 : b);
    }
    return IspError::OK;
}
