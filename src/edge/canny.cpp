#include "edge/canny.hpp"
#include "filter/gaussian_blur.hpp"
#include "edge/sobel.hpp"
IspError canny(const uint8_t* gray, uint8_t* edges, uint16_t w, uint16_t h, CannyConfig cfg) {
    (void)gray; (void)edges; (void)w; (void)h; (void)cfg;
    // TODO: 1. Gaussian blur 2. Sobel gradient 3. NMS 4. Hysteresis
    return IspError::OK;
}
