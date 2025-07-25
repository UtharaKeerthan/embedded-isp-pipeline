#include "edge/sobel.hpp"
IspError sobel(const uint8_t* gray, uint8_t* mag, uint16_t* dir, uint16_t w, uint16_t h) {
    if (!gray || !mag) return IspError::NULL_BUFFER;
    for (uint16_t r = 1; r < h-1; r++) {
        for (uint16_t c = 1; c < w-1; c++) {
            int32_t idx = r*w + c;
            int32_t gx = -gray[idx-w-1] + gray[idx-w+1]
                         -2*gray[idx-1] + 2*gray[idx+1]
                         -gray[idx+w-1] + gray[idx+w+1];
            int32_t gy = -gray[idx-w-1] - 2*gray[idx-w] - gray[idx-w+1]
                         +gray[idx+w-1] + 2*gray[idx+w] + gray[idx+w+1];
            int32_t m = (gx < 0 ? -gx : gx) + (gy < 0 ? -gy : gy); // L1 norm
            mag[idx] = (m > 255) ? 255 : (uint8_t)m;
            if (dir) dir[idx] = 0; // TODO: atan2 approximation
        }
    }
    return IspError::OK;
}
