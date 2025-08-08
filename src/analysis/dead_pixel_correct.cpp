#include "analysis/dead_pixel_correct.hpp"
IspError dead_pixel_correct(uint16_t* bayer, uint16_t w, uint16_t h,
                             const uint32_t* defect_map, uint32_t count) {
    if (!bayer || !defect_map) return IspError::NULL_BUFFER;
    for (uint32_t d = 0; d < count; d++) {
        uint32_t pos = defect_map[d];
        uint16_t r = (uint16_t)(pos / w), c = (uint16_t)(pos % w);
        if (r < 1 || r >= h-1 || c < 1 || c >= w-1) continue;
        uint32_t sum = bayer[pos-1] + bayer[pos+1] + bayer[pos-w] + bayer[pos+w];
        bayer[pos] = (uint16_t)(sum >> 2);
    }
    return IspError::OK;
}
