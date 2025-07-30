#pragma once
#include "types.hpp"
struct LensCoeffs { float k1; float k2; float k3; };
IspError distortion_correct(uint8_t* buf, uint16_t w, uint16_t h, LensCoeffs c);
