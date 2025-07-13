#include "color/hist_equalize.hpp"
#include <cstring>
IspError hist_equalize(uint8_t* gray, uint16_t w, uint16_t h) {
    if (!gray) return IspError::NULL_BUFFER;
    uint32_t hist[256] = {0};
    const size_t n = (size_t)w * h;
    for (size_t i = 0; i < n; i++) hist[gray[i]]++;
    uint32_t cdf[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) cdf[i] = cdf[i-1] + hist[i];
    uint32_t cdf_min = 0;
    for (int i = 0; i < 256; i++) { if (cdf[i] > 0) { cdf_min = cdf[i]; break; } }
    uint8_t lut[256];
    for (int i = 0; i < 256; i++) {
        uint32_t v = (uint32_t)(((float)(cdf[i] - cdf_min) / (float)(n - cdf_min)) * 255.0f);
        lut[i] = (v > 255) ? 255 : (uint8_t)v;
    }
    for (size_t i = 0; i < n; i++) gray[i] = lut[gray[i]];
    return IspError::OK;
}
IspError clahe(uint8_t* gray, uint16_t w, uint16_t h, ClaheConfig cfg) {
    (void)gray; (void)w; (void)h; (void)cfg;
    return IspError::OK; // TODO: tile-based CLAHE
}
