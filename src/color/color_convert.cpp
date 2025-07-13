#include "color/color_convert.hpp"
// BT.709 coefficients in Q1.15 fixed-point
IspError rgb_to_yuv_bt709(const uint8_t* rgb, uint8_t* yuv, uint16_t w, uint16_t h) {
    if (!rgb || !yuv) return IspError::NULL_BUFFER;
    const size_t n = (size_t)w * h;
    for (size_t i = 0; i < n; i++) {
        int32_t r = rgb[i*3], g = rgb[i*3+1], b = rgb[i*3+2];
        yuv[i*3]   = (uint8_t)((( 66*r + 129*g +  25*b + 128) >> 8) + 16);  // Y
        yuv[i*3+1] = (uint8_t)(((-38*r -  74*g + 112*b + 128) >> 8) + 128); // Cb
        yuv[i*3+2] = (uint8_t)(((112*r -  94*g -  18*b + 128) >> 8) + 128); // Cr
    }
    return IspError::OK;
}
IspError rgb_to_hsv(const uint8_t* rgb, uint8_t* hsv, uint16_t w, uint16_t h) {
    (void)rgb; (void)hsv; (void)w; (void)h;
    return IspError::OK; // TODO
}
