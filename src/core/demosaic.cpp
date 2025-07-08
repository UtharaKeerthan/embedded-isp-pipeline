#include "core/demosaic.hpp"
#include <cstddef>

/**
 * @tc TC-ISP-001
 * @covers REQ-ISP-001
 */
IspError demosaic_rggb(const uint16_t* bayer, uint8_t* rgb,
                        uint16_t width, uint16_t height) {
    if ((bayer == nullptr) || (rgb == nullptr)) { return IspError::NULL_BUFFER; }
    if ((width < 2U) || (height < 2U))          { return IspError::INVALID_SIZE; }

    // Bilinear interpolation for RGGB Bayer pattern
    // Pattern repeats in 2x2 blocks:
    //   R G
    //   G B
    for (uint16_t row = 1U; row < (height - 1U); row++) {
        for (uint16_t col = 1U; col < (width - 1U); col++) {
            const size_t idx     = static_cast<size_t>(row) * width + col;
            const size_t out_idx = idx * 3U;

            const bool is_even_row = ((row & 1U) == 0U);
            const bool is_even_col = ((col & 1U) == 0U);

            uint8_t r_val = 0U;
            uint8_t g_val = 0U;
            uint8_t b_val = 0U;

            if (is_even_row && is_even_col) {
                // Red pixel: R is known, G and B interpolated
                r_val = static_cast<uint8_t>(bayer[idx] >> 2U);
                g_val = static_cast<uint8_t>(
                    (bayer[idx - 1U] + bayer[idx + 1U] +
                     bayer[idx - width] + bayer[idx + width]) >> 4U);
                b_val = static_cast<uint8_t>(
                    (bayer[idx - width - 1U] + bayer[idx - width + 1U] +
                     bayer[idx + width - 1U] + bayer[idx + width + 1U]) >> 4U);
            } else if (is_even_row && !is_even_col) {
                // Green pixel on red row
                g_val = static_cast<uint8_t>(bayer[idx] >> 2U);
                r_val = static_cast<uint8_t>((bayer[idx - 1U] + bayer[idx + 1U]) >> 3U);
                b_val = static_cast<uint8_t>((bayer[idx - width] + bayer[idx + width]) >> 3U);
            } else if (!is_even_row && is_even_col) {
                // Green pixel on blue row
                g_val = static_cast<uint8_t>(bayer[idx] >> 2U);
                b_val = static_cast<uint8_t>((bayer[idx - 1U] + bayer[idx + 1U]) >> 3U);
                r_val = static_cast<uint8_t>((bayer[idx - width] + bayer[idx + width]) >> 3U);
            } else {
                // Blue pixel
                b_val = static_cast<uint8_t>(bayer[idx] >> 2U);
                g_val = static_cast<uint8_t>(
                    (bayer[idx - 1U] + bayer[idx + 1U] +
                     bayer[idx - width] + bayer[idx + width]) >> 4U);
                r_val = static_cast<uint8_t>(
                    (bayer[idx - width - 1U] + bayer[idx - width + 1U] +
                     bayer[idx + width - 1U] + bayer[idx + width + 1U]) >> 4U);
            }

            rgb[out_idx]     = r_val;
            rgb[out_idx + 1U] = g_val;
            rgb[out_idx + 2U] = b_val;
        }
    }
    return IspError::OK;
}
