#include "filter/median_filter.hpp"
IspError median_filter(uint8_t* buf, uint16_t w, uint16_t h, uint8_t r) {
    (void)buf; (void)w; (void)h; (void)r;
    return IspError::OK; // TODO: median via partial sort
}
