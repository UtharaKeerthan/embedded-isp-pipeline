#include "core/pipeline.hpp"
#include "core/memory_pool.hpp"
#include "core/demosaic.hpp"
#include "core/white_balance.hpp"
#include "core/gamma.hpp"
#include "analysis/dead_pixel_correct.hpp"
#include "color/ccm.hpp"
#include "color/color_convert.hpp"
#include "filter/gaussian_blur.hpp"
#include "edge/sobel.hpp"

IspError IspPipeline::init(float gamma_val) {
    return gamma_init_lut(gamma_val);
}

IspError IspPipeline::run(const uint16_t* bayer_in, uint8_t* yuv_out,
                           uint16_t width, uint16_t height) {
    if ((bayer_in == nullptr) || (yuv_out == nullptr)) {
        return IspError::NULL_BUFFER;
    }
    MemoryPool& pool = MemoryPool::instance();

    // Allocate intermediate buffers from pool
    uint16_t* corrected = static_cast<uint16_t*>(
        pool.alloc(static_cast<size_t>(width) * height * sizeof(uint16_t)));
    uint8_t* rgb = static_cast<uint8_t*>(
        pool.alloc(static_cast<size_t>(width) * height * 3U));

    if ((corrected == nullptr) || (rgb == nullptr)) {
        return IspError::POOL_EXHAUSTED;
    }

    // Stage 1: Dead pixel correction
    // TODO: load defect map from calibration data
    // dead_pixel_correct(corrected, width, height, defect_map, defect_count);

    // Stage 2: Demosaic
    IspError err = demosaic_rggb(bayer_in, rgb, width, height);
    if (err != IspError::OK) { return err; }

    // Stage 3: White balance
    WbGains gains = wb_compute_gray_world(rgb, width, height);
    err = wb_apply(rgb, width, height, gains);
    if (err != IspError::OK) { return err; }

    // Stage 4: Gamma correction
    err = gamma_apply(rgb, static_cast<size_t>(width) * height);
    if (err != IspError::OK) { return err; }

    // Stage 5: RGB -> YUV (BT.709)
    err = rgb_to_yuv_bt709(rgb, yuv_out, width, height);
    return err;
}
