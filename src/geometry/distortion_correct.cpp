#include "geometry/distortion_correct.hpp"
IspError distortion_correct(uint8_t* buf, uint16_t w, uint16_t h, LensCoeffs c) {
    (void)buf; (void)w; (void)h; (void)c;
    return IspError::OK; // TODO: polynomial radial correction
}
