FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    qemu-user-static \
    cmake \
    ninja-build \
    libgtest-dev \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install pillow numpy

WORKDIR /src
COPY . .

# Build for ARM (cross-compile)
RUN cmake -B build-arm \
    -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DSIL_MODE=ON

RUN cmake --build build-arm --parallel 4

# The ARM binary can be run transparently via QEMU on this x86 image
CMD ["./build-arm/isp_pipeline", "--help"]
