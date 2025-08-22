# knowledge.md — Technical Dictionary
### embedded-isp-pipeline

Reference guide for every technical term, algorithm, standard, and concept
used in this project. Intended for developers new to embedded camera systems,
for onboarding, and as the RAG source document for the EmbedSentinel AI agents.

---

## Part 1 — What is a BSP and an ISP?

### BSP — Board Support Package

A BSP is the layer of software that makes a general-purpose operating system
or application work on a specific piece of hardware. It contains device
drivers, boot loaders, pin multiplexing configuration, and hardware
abstraction code tailored to one specific board or chip — for example, the
NXP i.MX8QM evaluation kit.

Think of a BSP as the translator between generic software and the specific
pins, buses, memory maps, and peripherals on a physical board. Without a BSP,
an operating system has no way to talk to a camera sensor, a CAN bus
controller, or a display output.

In the context of this project, the ISP pipeline is a component that would
live within a BSP's camera subsystem, sitting between the raw sensor driver
and the application layer that processes or displays video.

### ISP — Image Signal Processor

An ISP is a dedicated processor or software pipeline that transforms the raw
output of a camera sensor into a viewable, high-quality image. Every
smartphone, dashcam, and automotive camera contains one. The raw output of a
CMOS sensor is a mosaic of single-channel samples (see Bayer Pattern) that
looks like a grainy, greenish noise pattern — it must be processed through
several stages before it becomes a recognisable photograph.

This project implements those stages in software, simulating what hardware
ISPs do inside ECUs. Running the ISP in software rather than dedicated silicon
allows development and validation without physical hardware.

---

## Part 2 — The Camera Sensor and Raw Data

### CMOS Sensor

A CMOS (Complementary Metal-Oxide-Semiconductor) image sensor captures light
using millions of tiny photodetectors called pixels. Each pixel measures light
intensity as an analogue voltage, which is then digitised to a numeric value.
Crucially, a standard CMOS pixel cannot inherently distinguish colour — it
only measures the total quantity of light, not its wavelength.

### CFA — Color Filter Array

To capture colour, a physical filter array is placed over the sensor grid.
Each pixel is covered by one coloured filter — Red, Green, or Blue. The
filter allows only light of that colour to reach the photodetector beneath it.
The resulting raw image is therefore a mosaic of single-colour samples: each
pixel knows its one colour's intensity, and the other two colours must be
estimated by the ISP using neighbouring pixels.

### Bayer Pattern (RGGB)

The most common CFA arrangement, invented by Bryce Bayer at Kodak in 1976.
The pattern tiles across the sensor in repeating 2×2 blocks:

```
R  G  R  G  R  G  R  G
G  B  G  B  G  B  G  B
R  G  R  G  R  G  R  G
G  B  G  B  G  B  G  B
```

There are twice as many Green samples as Red or Blue because the human visual
system is most sensitive to green wavelengths. Doubling the green samples
improves luminance resolution and reduces the appearance of noise in the
reconstructed image.

The name "RGGB" describes the layout of the top-left 2×2 tile. Other
variants exist depending on which colour appears in the top-left corner:
GRBG, BGGR, and GBRG. The variant depends on the physical sensor design and
must be known to the ISP for correct demosaicing.

### Raw Bayer Buffer

The binary data produced by the sensor after a complete frame is captured.
Each value is a single integer — typically 10-bit or 12-bit — representing one
colour channel's light intensity at one pixel location. In this project raw
Bayer data is stored as 16-bit unsigned integers (`uint16_t`) in row-major
order. The file format is a flat binary with no header: simply
`width × height × 2` bytes.

In a real ECU, this buffer is filled automatically by the camera sensor's
DMA controller at each frame interval and placed directly into a pre-allocated
region of system memory.

### DMA — Direct Memory Access

A hardware mechanism that allows peripheral devices — such as a camera
sensor controller — to write data directly into system memory without
involving the CPU for each byte transferred. The CPU only needs to configure
the DMA channel once; the DMA controller then handles the bulk transfer
autonomously at hardware speed.

In a real ECU the camera sensor's DMA controller fills a frame buffer at
1920×1080×2 bytes = approximately 4 MB per frame, 30 times per second — a
data rate that would saturate the CPU if done with software loops. In our
SIL simulation we mimic this by reading a `.raw` file into a pre-allocated
static buffer at startup.

---

## Part 3 — ISP Pipeline Stages Explained

### Stage 1 — Dead Pixel Correction

**The problem:** All CMOS sensors contain a small number of permanently
defective pixels. Hot pixels always output a very high value regardless of
scene brightness. Dead pixels always output near zero. Without correction
these appear as fixed bright or dark spots in every frame.

**The solution:** During factory calibration, the sensor's defect map is
measured and stored in the ECU's non-volatile memory. At runtime, before any
other processing, each defective pixel's value is replaced by the median of
its four orthogonal neighbours (up, down, left, right). Applying this before
demosaicing prevents a single bad sample from corrupting a larger area during
interpolation.

---

### Stage 2 — Demosaicing (CFA Interpolation)

**The problem:** Each pixel in the raw buffer contains a value for only one
colour channel. To produce a displayable RGB image, all three channels must
be known at every pixel location.

**The solution:** Estimate the missing two channels at each pixel by
interpolating from neighbouring pixels of those colours. This is called
demosaicing or CFA interpolation.

**Bilinear interpolation** (used in this project): For a Red pixel in an
RGGB sensor, the missing Green value is estimated by averaging the four
adjacent Green pixels. The missing Blue value is estimated by averaging the
four diagonal Blue pixels. This is computationally simple, MISRA-safe, and
has predictable, cache-friendly memory access patterns.

More advanced methods such as AHD (Adaptive Homogeneity-Directed) and VNG
(Variable Number of Gradients) produce sharper results at colour edges but
require significantly more computation. Bilinear is appropriate for
automotive ECUs where determinism and cycle count matter more than
photographic quality.

---

### Stage 3 — White Balance

**The problem:** Different light sources emit different spectral distributions.
Sunlight, fluorescent lamps, and tungsten bulbs each give a scene a different
colour cast. An object that is white in daylight appears orange under tungsten
light if the camera does not compensate. The human visual system corrects for
this automatically (chromatic adaptation), but a camera sensor records the
raw physical light and needs an explicit correction.

**The solution:** Multiply each colour channel by a calibration gain so that
objects that should appear neutral (white, grey) actually appear neutral.

**Gray World Algorithm** (used in this project): Assumes that the average
colour of a natural scene is approximately neutral grey — that is, the mean
values of R, G, and B channels across the frame should be equal. The
algorithm computes these means and sets per-channel gains to equalise them.
Simple, fast, and MISRA-compatible because it uses only integer arithmetic.

The gains are stored as Q8.8 fixed-point integers (see Fixed-Point
Arithmetic).

---

### Stage 4 — Color Correction Matrix (CCM)

**The problem:** Even after white balance, camera sensors do not capture
colour with the same accuracy as the human visual system. The sensor's
spectral sensitivity curves do not perfectly match the human eye's cone
response curves, causing a systematic colour error: skin tones may look too
yellow, blues may look too cyan.

**The solution:** Apply a 3×3 calibration matrix that linearly transforms
the sensor's RGB values into a standard colour space (such as sRGB for
displays, or a vehicle manufacturer's display profile).

```
| R_out |   | a00  a01  a02 |   | R_in |
| G_out | = | a10  a11  a12 | × | G_in |
| B_out |   | a20  a21  a22 |   | B_in |
```

The nine matrix coefficients are determined during sensor characterisation
at the factory and stored in the ECU's flash. In this project they are stored
as Q4.12 fixed-point integers, which gives sufficient precision for the small
correction values involved (most coefficients are close to 1.0 or 0.0).

---

### Stage 5 — Noise Reduction

**The problem:** Sensor noise causes random variations in pixel values.
In low light or high-gain conditions, raw images appear visually grainy. Two
types dominate: photon shot noise (statistical variation in photon arrival)
and read noise (electronic noise in the sensor's amplifier circuit).

**Gaussian Blur:** Averages each pixel with its spatial neighbours using
a bell-curve-shaped weight kernel. Pixels close to the centre contribute
more to the average than those further away. The kernel is separable — a 2D
Gaussian blur can be decomposed into one 1D horizontal pass followed by one
1D vertical pass, halving the number of multiply-accumulate operations.
Smooths noise but also blurs sharp edges.

**Bilateral Filter** (used in this project for noise reduction): A more
intelligent blur that weights neighbours by both spatial distance AND
colour similarity. Pixels with a colour close to the centre pixel contribute
strongly to the average; pixels across a colour edge contribute little or
nothing. This preserves edges while smoothing uniform regions. More
computationally expensive than Gaussian but produces better results for
automotive camera applications where edge sharpness matters.

**Median Filter:** Replaces each pixel with the median value of all pixels
in a small neighbourhood window. Extremely effective at removing salt-and-
pepper noise (isolated black or white pixels) without blurring edges. Uses
sorting rather than linear convolution, making it a non-linear operation.

---

### Stage 6 — Gamma Correction

**The problem:** Camera sensors respond linearly to light: twice the photons
produce twice the pixel value. Human eyes respond logarithmically: twice the
physical brightness looks only slightly brighter, and very dark areas are
perceived with much more sensitivity than linear encoding would suggest. A
linear image displayed on a monitor appears too dark in the shadows and
wastes encoding range on bright highlights that the eye barely distinguishes.

**The solution:** Apply a power-law transfer function to remap pixel values
from the linear sensor domain to a perceptually uniform display domain:

```
output = input ^ (1 / gamma)
```

A gamma value of 2.2 is standard for sRGB displays. This brightens the
shadows (where the eye is sensitive) and compresses the highlights (where
the eye is less sensitive).

**LUT-based implementation** (used in this project): Computing the power
function at runtime requires floating-point arithmetic, which is banned under
MISRA C++ and unavailable on ECUs without a hardware FPU. Instead, all 256
possible output values are pre-computed offline and stored in a 256-entry
`uint8_t` lookup table (LUT). At runtime, the mapping is a simple array
index: `output = gamma_lut[input]`. This is O(1) per pixel, purely integer,
and entirely deterministic.

---

### Stage 7 — Histogram Equalization

**The problem:** An image with poor dynamic range — where most pixel values
are clustered in a narrow band — appears washed out or lacking in contrast.
This can happen in fog, overcast conditions, or tunnels.

**The solution:** Redistribute pixel intensities so they span the full
available range (0–255) more evenly.

**Algorithm:**
1. Compute histogram: count how many pixels have each intensity value (0–255).
2. Compute the cumulative distribution function (CDF) of the histogram.
3. Use the CDF as a remapping function: `new_intensity = CDF(old_intensity) × 255`.

Pixels in crowded intensity regions are spread apart; pixels in sparse regions
are compressed. The result has higher perceived contrast.

**CLAHE — Contrast Limited Adaptive Histogram Equalization** (also
implemented): Divides the image into a grid of small tiles and equalises
each tile's histogram independently. Then blends tile boundaries smoothly
using bilinear interpolation. This prevents over-amplification of noise in
uniform regions (a problem with global histogram equalisation) and adapts
contrast enhancement to local image content. Tile size is configurable via
`REQ-HIST-002`.

---

### Stage 8 — Sharpening (Unsharp Mask)

**The problem:** Demosaicing and noise reduction both soften fine detail and
edges. The output image may look slightly blurry compared to what the
scene actually looked like.

**The solution — Unsharp Mask:**
1. Blur the image with a Gaussian filter to produce a low-frequency copy.
2. Subtract the blur from the original to isolate high-frequency content
   (edges and fine detail): `edge = original - blurred`.
3. Add a scaled version of the edge signal back to the original:
   `sharpened = original + strength × edge`.

The paradoxical name comes from the original photographic technique, where
a blurred (unsharp) physical negative was used as a mask to emphasise
the sharp original print.

**Laplacian sharpening** (alternative): Applies a Laplacian kernel that
directly computes the second derivative of image intensity, highlighting
regions of rapid change (edges). Added back to the original with a small
gain factor.

---

### Stage 9 — Edge Detection

**The problem:** Many downstream applications — lane detection, object
detection preprocessing, visual odometry — need to identify the boundaries
of objects in an image.

**Sobel Filter:**
Computes the image gradient — the rate at which pixel values change — in
the horizontal (X) and vertical (Y) directions by convolving with two
3×3 kernels:

```
Gx = | -1   0  +1 |     Gy = | -1  -2  -1 |
     | -2   0  +2 |          |  0   0   0 |
     | -1   0  +1 |          | +1  +2  +1 |
```

The gradient magnitude `G = sqrt(Gx² + Gy²)` is high at edges and low in
flat regions. The gradient direction `θ = atan2(Gy, Gx)` indicates which
direction the edge runs. In this project, `sqrt` is approximated by the
cheaper `G = |Gx| + |Gy|` (L1 norm) to avoid floating-point on ECUs.

**Canny Edge Detector** (multi-stage pipeline):
The most widely used edge detection algorithm. Four sequential stages:

1. **Gaussian blur** — suppress sensor noise before computing gradients,
   as the Sobel operator is sensitive to noise.
2. **Sobel gradient** — compute magnitude and direction at every pixel.
3. **Non-Maximum Suppression (NMS)** — thin wide edge bands to single-pixel
   width. For each pixel, check the two neighbours along the gradient
   direction. If the pixel is not a local maximum compared to those two
   neighbours, suppress it (set to zero). This eliminates the thick, blurry
   edges produced by the raw Sobel response.
4. **Hysteresis thresholding** — apply two thresholds: high (`T_high`) and
   low (`T_low`). Pixels above `T_high` are definite edges. Pixels between
   `T_low` and `T_high` are edges only if they are connected (8-connected)
   to a definite edge. Pixels below `T_low` are discarded as noise. The two-
   threshold approach makes the result robust to noise while still connecting
   faint edges that are part of a real boundary.

Both `T_high` and `T_low` are configurable via `REQ-EDGE-003`.

---

### Stage 10 — Geometric Processing

**Image Resize (Bilinear Interpolation):**
Scales an image to a different resolution. For each output pixel, compute
its fractional location in the input image. The output value is a weighted
average of the four nearest input pixels (the 2×2 neighbourhood surrounding
the fractional location), weighted by the area each input pixel occupies at
the fractional location. The result is smooth and avoids the jagged
appearance of nearest-neighbour scaling.

**Lens Distortion Correction:**
Real lenses introduce geometric distortion because the optical magnification
varies with distance from the optical axis. Barrel distortion (common in
wide-angle and fisheye lenses) bows straight lines outward toward the image
edges. Pincushion distortion bows them inward. For ADAS applications, where
accurate spatial measurements are derived from camera images, distortion must
be corrected before processing.

Correction uses a polynomial model derived from calibration:

```
r_corrected = r_distorted × (1 + k1×r² + k2×r⁴ + k3×r⁶)
```

where `r` is the radial distance from the optical centre and `k1`, `k2`, `k3`
are the lens distortion coefficients measured during calibration. Each output
pixel's corrected location is computed and used to sample the input image with
bilinear interpolation.

---

### Stage 11 — Auto-Exposure

**The problem:** Scene brightness varies widely as a vehicle drives from a
tunnel into sunlight. A fixed camera exposure setting would produce
over-exposed or under-exposed images in many conditions.

**The solution:** An iterative control algorithm that adjusts the camera's
exposure time and analogue gain each frame to maintain a target mean
luminance in the output image.

**Algorithm (proportional controller):**

Each frame:
1. Measure current mean luminance `Y_current` by averaging the Y channel
   after colour space conversion.
2. Compute the error: `e = Y_target - Y_current`.
3. Adjust exposure: `exp_new = exp_old × (1 + k × e)`, where `k` is the
   controller gain constant.
4. Clamp to hardware limits: minimum and maximum exposure time and gain.
5. Send the new exposure value to the sensor driver via I2C.

`REQ-AE-001` requires convergence to `Y_target` within 10 frames for
typical scene transitions.

---

### Stage 12 — Color Space Conversion

**RGB (Red-Green-Blue):**
The additive colour model used by displays. Each pixel is represented by
three 8-bit values (0–255) for red, green, and blue intensity. The three
channels combined additively produce the perceived colour.

**YUV / YCbCr:**
Separates luminance (Y, brightness) from chrominance (Cb = blue-difference,
Cr = red-difference). Because the human visual system is far more sensitive
to brightness variations than colour variations, YUV is used in all video
compression standards (JPEG, H.264, H.265, HEVC) to apply more compression
to the colour channels than the luminance channel without visible quality loss.

BT.709 defines the coefficients used for high-definition video and is the
standard for automotive displays:

```
Y  =  0.2126 R + 0.7152 G + 0.0722 B
Cb = -0.1146 R - 0.3854 G + 0.5000 B
Cr =  0.5000 R - 0.4542 G - 0.0458 B
```

In this project, these coefficients are stored as Q1.15 fixed-point integers.

**HSV (Hue-Saturation-Value):**
Represents colour as a hue angle on a colour wheel (0°–360°), a saturation
percentage (0% = grey, 100% = fully saturated), and a brightness value
(0 = black, 100% = full brightness). HSV is more intuitive than RGB for
colour-based image analysis: detecting orange traffic cones, for example, is
trivially expressed as a range of hue values in HSV but requires a complex
condition on all three channels in RGB.

---

## Part 4 — Embedded Constraints

### Fixed-Point Arithmetic

Floating-point operations (`float`, `double`) are banned in this codebase
for two reasons: MISRA C++ discourages them in safety-critical systems due to
rounding behaviour and NaN/infinity propagation, and many ECU-class processors
lack a hardware floating-point unit (FPU), making software float emulation
too slow for real-time image processing.

Fixed-point arithmetic represents fractional values using integers by choosing
a fixed position for the binary (decimal) point.

**Q8.8 format:**
A 16-bit integer where the upper 8 bits represent the integer part and the
lower 8 bits represent the fractional part. The representable range is
-128.0 to +127.996 with a resolution of 1/256 ≈ 0.0039.

The value `1.5` is stored as `1.5 × 256 = 384 = 0x0180`.

To multiply two Q8.8 numbers: multiply them as `int32_t` (which gives a Q16.16
result), then arithmetic right-shift by 8 bits to return to Q8.8. No
floating-point hardware is needed.

White balance gain values are stored in Q8.8 in this project.

**Q4.12 format:**
A 16-bit integer with 4 integer bits and 12 fractional bits. Covers the range
-8.0 to +7.9998 with resolution 1/4096 ≈ 0.00024. Used for CCM coefficients,
which are small values close to 0 or 1 that need high fractional precision.

**Q1.15 format:**
15 fractional bits, covering -1.0 to +0.99997. Used for BT.709 YUV
coefficients which are all in the range [-1, +1].

---

### Static Memory Allocation

Dynamic memory allocation (`malloc`, `new`) is banned under MISRA C++
Rule 18-4-1 and is generally avoided in safety-critical embedded software
for three reasons:

1. **Heap fragmentation:** repeated allocation and deallocation of variable-
   sized blocks fragments the heap over time, eventually causing `malloc` to
   fail even when total free memory is sufficient.
2. **Non-deterministic timing:** the time taken by `malloc` depends on the
   current state of the heap and varies unpredictably. Real-time systems with
   strict timing budgets cannot tolerate this.
3. **NULL returns:** `malloc` returns `NULL` on failure, requiring an error
   check at every call site. Missed checks cause null pointer dereferences.

**Static memory pool** (used in this project): A large `uint8_t` array
allocated at compile time in the BSS segment. Sub-allocations come from this
pool via a simple bump pointer that advances by the requested size. The
allocator is O(1), deterministic, and cannot fragment. The pool is sized
at compile time to the maximum memory any pipeline configuration will ever
need.

```cpp
static uint8_t   pool[FRAME_BUF_SIZE];   // compile-time allocation in BSS
static size_t    pool_offset = 0;

void* pool_alloc(size_t size) {
    void* ptr = &pool[pool_offset];
    pool_offset += size;                  // O(1), no fragmentation
    return ptr;
}
```

The pool is reset to offset zero at the start of each frame, reusing the
same memory for every frame.

---

### Stack Usage

The GCC compiler flag `-Wstack-usage=4096` causes a warning when any
function's local variable storage (stack frame) exceeds 4096 bytes. On
embedded systems, the total stack is often limited to 8–32 KB of SRAM.
Functions with large local arrays or deep call chains can overflow the
stack, corrupting adjacent memory regions in ways that are difficult to
debug. `REQ-MEM-001` enforces the 4096-byte limit for all functions in
this project.

---

## Part 5 — Safety Standards

### MISRA C++:2008

A set of C++ coding guidelines published by the Motor Industry Software
Reliability Association. Prohibits or restricts C++ language features that
can lead to undefined behaviour, implementation-defined behaviour, or
unpredictable execution in embedded and safety-critical contexts.

Key rules relevant to this project:

| Rule | Category | Requirement |
|---|---|---|
| 18-4-1 | Required | Dynamic heap memory shall not be used |
| 15-0-1 | Mandatory | Exceptions shall not be used |
| 5-0-8 | Required | Explicit casts shall not decrease precision |
| 7-5-4 | Required | Functions shall not call themselves directly or indirectly |
| 6-4-1 | Required | All if-else chains shall have a final else clause |
| 8-4-4 | Required | A function identifier shall either be used to call the function or it shall be preceded by & |

The EmbedSentinel MISRA agent checks code against these rules automatically
on every pull request.

---

### ISO 26262

The international functional safety standard for road vehicles
(IEC 61508 adapted for automotive). Defines ASIL (Automotive Safety
Integrity Level) from QM (no safety requirement) through ASIL-A, B, C to
ASIL-D (the most stringent). The ASIL level is determined by analysing the
severity, exposure, and controllability of a potential hazard.

Relevant to software (Part 6):

- **ASIL-A/B:** Source code must be reviewed; unit tests required; statement
  and branch coverage required.
- **ASIL-C:** MC/DC (Modified Condition/Decision Coverage) testing required;
  formal design review.
- **ASIL-D:** Strongest requirements; formal methods or diverse redundancy
  may be needed; 100% MC/DC coverage.

The EmbedSentinel safety agent classifies each ISP pipeline stage's ASIL
level and flags gaps in testing coverage relative to that classification.

---

### ASPICE — Automotive SPICE

A software process assessment model based on ISO/IEC 15504, widely used
across the automotive supply chain. Defines capability levels (0–5) for
software development processes. Tier-1 suppliers to OEMs are typically
required to achieve ASPICE Level 2 or 3.

Relevant to this project because ASPICE mandates:
- **SWE.1:** Software requirements — each functional requirement has a
  unique identifier (the REQ-XXX-NNN scheme used in SRS.md).
- **SWE.4:** Software unit verification — each unit has associated test
  cases (the @tc annotation scheme).
- **SWE.5 / SWE.6:** Traceability from requirements to design to code to
  tests (the traceability matrix generated by EmbedSentinel).

---

## Part 6 — Development Methodology

### SIL — Software-in-the-Loop

A validation technique where the software under test runs on a standard
development PC — the 'loop' — with real hardware inputs and outputs replaced
by simulated equivalents. In this project:

| Real ECU component | SIL simulation |
|---|---|
| Camera sensor DMA output | Synthetic Bayer `.raw` file (Python generator) |
| ECU hardware (i.MX8) | x86 development PC |
| ARM Cortex-A processor | QEMU ARM emulator |
| No-malloc, no-float constraints | Enforced by compile flags and static allocator |
| ECU frame buffer | Pre-allocated static `uint8_t` array |

SIL is defined in ISO 26262 Part 4 as a valid first validation tier before
Hardware-in-the-Loop (HIL) testing on real ECU hardware. Running SIL first
finds the majority of algorithmic defects cheaply, before expensive hardware
time is needed.

---

### HIL — Hardware-in-the-Loop

A validation technique where the real ECU hardware runs the production
firmware with external sensors and actuators replaced by a simulation
platform (typically a real-time PC with analogue and digital I/O). HIL
validates the integration of software with hardware in a controlled
environment before vehicle testing. It is the next step after SIL and
requires the physical ECU board. Not used in this project.

---

### QEMU — Quick EMUlator

An open-source machine emulator and virtualiser that can emulate a wide
range of processor architectures. In this project, QEMU emulates an ARM
Cortex-A class processor (aarch64 architecture) on an x86 laptop.

After cross-compiling the ISP pipeline binary for ARM using the
`aarch64-linux-gnu-g++` cross-compiler, the resulting ARM binary is executed
transparently by QEMU. The developer runs the binary with a normal command;
QEMU intercepts each ARM instruction, translates it to the equivalent x86
instruction sequence, and executes it on the host CPU. The translation
overhead means QEMU runs slower than real hardware, but functionally the
behaviour is identical.

This makes it possible to:
- Run and debug the ARM binary without any ARM hardware.
- Execute the binary in CI (GitHub Actions runners are x86).
- Verify that cross-compilation produces a correctly functioning binary.

---

### Cross-Compilation

The process of compiling source code on one processor architecture (the
*host*, typically x86) to produce a binary that executes on a different
processor architecture (the *target*, typically ARM in this project).

Requires a cross-compiler toolchain: a version of the compiler, linker,
and standard libraries built to run on x86 but produce ARM output. In this
project, `aarch64-linux-gnu-g++` from the GNU toolchain is used.

The `cmake/aarch64-toolchain.cmake` file tells CMake to use the cross-
compiler and to link against the ARM sysroot (the collection of ARM-
compiled standard libraries).

---

### ARM Cortex-A and NEON SIMD

The ARM Cortex-A processor family is used in automotive ECUs (NXP i.MX8,
S32G) and high-performance embedded devices. It implements the ARMv8 64-bit
instruction set (AArch64).

**NEON** is the ARM SIMD (Single Instruction, Multiple Data) extension. NEON
registers are 128 bits wide and can be interpreted as:
- 16 × 8-bit integers
- 8 × 16-bit integers
- 4 × 32-bit integers

A single NEON multiply-accumulate instruction (`VMLA`) can process 16 pixels
simultaneously. Inner loops in demosaicing, convolution, and colour conversion
are the primary candidates for NEON vectorisation.

The EmbedSentinel Performance Agent identifies inner loops that are not yet
vectorised and suggests NEON intrinsic replacements.

---

## Part 7 — OAK-D Camera Terms

(Relevant for the companion `oak-d-driver-monitor` project.)

### VPU — Vision Processing Unit

The Intel Myriad X VPU inside the OAK-D camera. An ultra-low-power processor
optimised for neural network inference. Runs OpenVINO models compiled to
`.blob` format on-device, without requiring a GPU or heavy CPU on the host
machine. The OAK-D communicates with the host via USB 3.0, sending only the
final processed outputs (face bounding boxes, head pose angles, landmark
coordinates) rather than raw pixel data, minimising USB bandwidth.

### OpenVINO — Open Visual Inference and Neural network Optimization

Intel's open-source toolkit for neural network inference, optimised for
Intel hardware including CPUs, GPUs, FPGAs, and the Myriad X VPU. The
Model Zoo provides hundreds of pre-trained models in Intel's Intermediate
Representation (IR) format, which can be compiled to `.blob` files for the
Myriad X.

Models used in `oak-d-driver-monitor`:

| Model | Purpose |
|---|---|
| `face-detection-adas-0001` | Detects driver face bounding box, runs on VPU |
| `head-pose-estimation-adas-0001` | Outputs yaw, pitch, roll angles, runs on VPU |

### MediaPipe FaceMesh

Google's real-time facial landmark detection model. Given an RGB face image,
it outputs the 3D coordinates of 468 facial landmarks — including 7 points
per eyelid, the mouth corners, and the nose tip. Runs efficiently on a laptop
CPU in Python using TensorFlow Lite. Used in `oak-d-driver-monitor` for EAR
calculation and yawn detection.

### EAR — Eye Aspect Ratio

A scalar metric derived from 6 eye landmark coordinates that quantifies how
open the eye is. Calculated from the ratio of the eye's vertical span to its
horizontal span:

```
     ||P2-P6|| + ||P3-P5||
EAR = ─────────────────────
          2 × ||P1-P4||
```

Where P1–P6 are the 6 eye landmarks (corners and eyelid points). An open eye
has EAR ≈ 0.30. A fully closed eye has EAR ≈ 0.00. Drowsiness threshold: < 0.20.

EAR is invariant to uniform image scaling and in-plane face rotation, making
it robust across different driver positions and head angles.

### PERCLOS — Percentage of Eye Closure

The fraction of frames within a rolling time window (typically 60 seconds) in
which the driver's eyes are at least 80% closed. PERCLOS was developed by
researchers at Carnegie Mellon University and adopted by NHTSA as the
gold-standard objective metric for driver drowsiness.

```
PERCLOS = (frames where EAR < 0.06) / (total frames in 60s window)
```

PERCLOS > 0.80 is the threshold for drowsy classification in this project.

### DepthAI Pipeline

The OAK-D programming model. A directed acyclic graph of processing nodes
(camera inputs, neural network inference, stereo depth computation, etc.)
that is defined on the host in Python, serialised to JSON, and sent to the
OAK device where it executes on the VPU. The host receives only the final
outputs (bounding boxes, angles, landmark arrays), keeping USB bandwidth low
and host CPU usage minimal. The node graph design mirrors the pipeline
architecture of embedded ISP systems.

---

## Part 8 — Agentic AI Terms

(Relevant for the `embedsentinel` project.)

### RAG — Retrieval-Augmented Generation

A technique for making large language models more accurate and domain-
specific by retrieving relevant reference documents before generating a
response. Instead of relying solely on the LLM's training data (which may
be outdated or insufficiently detailed), the system first queries a vector
database for documents relevant to the current task, then provides those
documents as context in the LLM prompt.

In EmbedSentinel: the MISRA agent receives a code chunk → queries ChromaDB
for the most relevant MISRA rules → passes both the code and the rules to the
LLM → the LLM reasons about violations given the specific rules rather than
general knowledge.

### ChromaDB

An open-source vector database designed for embedding storage and semantic
similarity search. Stores text documents as high-dimensional embedding vectors
and retrieves the most semantically similar documents for a given query. Used
in EmbedSentinel as the backend for all RAG knowledge bases.

### CrewAI

An open-source Python framework for orchestrating multiple AI agents in a
collaborative workflow. Each agent has a role, a goal, a backstory, and a
set of tools. Tasks define what each agent must produce. The crew coordinates
task execution, passing outputs from one agent as inputs to the next.

### Sentence Transformers

A Python library providing pre-trained embedding models that convert text
into fixed-length numerical vectors. Semantically similar sentences produce
vectors that are close together in the embedding space. Used to generate the
embeddings stored in ChromaDB. Runs locally with no API key required.

### LLM — Large Language Model

A neural network trained on large quantities of text data to predict and
generate natural language. In EmbedSentinel, the LLM receives a code chunk
plus retrieved reference documents and generates a structured analysis
(violations, classifications, test skeletons). Ollama is used to run the
`codellama:7b` model locally, keeping all code and proprietary information
on-premises.
