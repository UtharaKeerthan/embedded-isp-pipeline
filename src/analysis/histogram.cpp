#include "analysis/histogram.hpp"
#include <cstring>
IspError compute_histogram(const uint8_t* ch, uint16_t w, uint16_t h, uint32_t* out) {
    if (!ch || !out) return IspError::NULL_BUFFER;
    memset(out, 0, 256 * sizeof(uint32_t));
    for (size_t i = 0; i < (size_t)w*h; i++) out[ch[i]]++;
    return IspError::OK;
}
