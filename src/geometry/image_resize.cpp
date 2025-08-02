#include "geometry/image_resize.hpp"
IspError bilinear_resize(const uint8_t* src, uint16_t sw, uint16_t sh,
                          uint8_t* dst, uint16_t dw, uint16_t dh, uint8_t ch) {
    (void)src; (void)sw; (void)sh; (void)dst; (void)dw; (void)dh; (void)ch;
    return IspError::OK; // TODO: bilinear interpolation
}
