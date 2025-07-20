#include "filter/sharpening.hpp"
IspError unsharp_mask(uint8_t* buf, uint16_t w, uint16_t h, float s) {
    (void)buf; (void)w; (void)h; (void)s;
    return IspError::OK; // TODO: blur then add edge signal
}
