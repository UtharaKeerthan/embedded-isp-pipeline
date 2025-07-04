#pragma once
#include <cstdint>
#include <cstddef>

constexpr uint16_t FRAME_WIDTH   = 1920U;
constexpr uint16_t FRAME_HEIGHT  = 1080U;
constexpr size_t   FRAME_PIXELS  = static_cast<size_t>(FRAME_WIDTH) * FRAME_HEIGHT;
constexpr size_t   FRAME_BUF_SIZE = FRAME_PIXELS * 2U;

using q8_8_t  = int16_t;   // Q8.8  fixed-point
using q4_12_t = int16_t;   // Q4.12 fixed-point
using q1_15_t = int16_t;   // Q1.15 fixed-point

constexpr q8_8_t float_to_q8_8(float v)  { return static_cast<q8_8_t>(v * 256.0F); }
constexpr float  q8_8_to_float(q8_8_t v) { return static_cast<float>(v) / 256.0F; }

enum class IspError : uint8_t {
    OK = 0U, NULL_BUFFER = 1U, INVALID_SIZE = 2U,
    POOL_EXHAUSTED = 3U, INVALID_PARAM = 4U
};
