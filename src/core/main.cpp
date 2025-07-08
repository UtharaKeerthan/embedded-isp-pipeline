#include "core/pipeline.hpp"
#include "core/memory_pool.hpp"
#include <cstdio>
#include <cstring>

static uint16_t s_bayer_buf[FRAME_PIXELS];
static uint8_t  s_yuv_out  [FRAME_PIXELS * 3U];

static bool read_raw_file(const char* path, uint16_t* buf, uint16_t w, uint16_t h) {
    FILE* f = fopen(path, "rb");
    if (f == nullptr) { return false; }
    const size_t expected = static_cast<size_t>(w) * h * sizeof(uint16_t);
    const size_t read     = fread(buf, 1U, expected, f);
    fclose(f);
    return (read == expected);
}

int main(int argc, const char* argv[]) {
    const char* input_path  = (argc > 1) ? argv[1] : "input/test_frame.raw";
    const char* output_path = (argc > 2) ? argv[2] : "output/result.ppm";

    if (!read_raw_file(input_path, s_bayer_buf, FRAME_WIDTH, FRAME_HEIGHT)) {
        printf("[ERROR] Could not read %s\n", input_path);
        return 1;
    }

    IspPipeline pipeline;
    IspError err = pipeline.init(2.2F);
    if (err != IspError::OK) { printf("[ERROR] Pipeline init failed\n"); return 1; }

    MemoryPool::instance().reset();
    err = pipeline.run(s_bayer_buf, s_yuv_out, FRAME_WIDTH, FRAME_HEIGHT);
    if (err != IspError::OK) { printf("[ERROR] Pipeline run failed: %d\n", static_cast<int>(err)); return 1; }

    printf("[OK] Processed %ux%u frame. Memory used: %zu / %zu bytes\n",
           FRAME_WIDTH, FRAME_HEIGHT,
           MemoryPool::instance().used(),
           MemoryPool::instance().capacity());
    return 0;
}
