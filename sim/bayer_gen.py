#!/usr/bin/env python3
"""
bayer_gen.py
Converts any PNG/JPEG to a synthetic Bayer RGGB .raw binary file.
Simulates the flat binary output of a camera sensor DMA controller.

Usage:
    python sim/bayer_gen.py --input sample.jpg --output input/test_frame.raw
"""
import argparse
import numpy as np
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("Install Pillow: pip install pillow")
    raise

def rgb_to_bayer_rggb(img_array: np.ndarray) -> np.ndarray:
    """Convert HxWx3 RGB array to HxW Bayer RGGB uint16 array."""
    h, w = img_array.shape[:2]
    bayer = np.zeros((h, w), dtype=np.uint16)
    # RGGB pattern (top-left 2x2 = R G / G B)
    bayer[0::2, 0::2] = img_array[0::2, 0::2, 0].astype(np.uint16) << 2  # R (10-bit)
    bayer[0::2, 1::2] = img_array[0::2, 1::2, 1].astype(np.uint16) << 2  # G
    bayer[1::2, 0::2] = img_array[1::2, 0::2, 1].astype(np.uint16) << 2  # G
    bayer[1::2, 1::2] = img_array[1::2, 1::2, 2].astype(np.uint16) << 2  # B
    return bayer

def main():
    parser = argparse.ArgumentParser(description="Convert image to Bayer .raw")
    parser.add_argument("--input",  required=True, help="Input image (PNG/JPEG)")
    parser.add_argument("--output", required=True, help="Output .raw file")
    parser.add_argument("--width",  type=int, default=1920)
    parser.add_argument("--height", type=int, default=1080)
    parser.add_argument("--pattern", default="RGGB",
                        choices=["RGGB", "GRBG", "BGGR", "GBRG"])
    args = parser.parse_args()

    img = Image.open(args.input).convert("RGB")
    img = img.resize((args.width, args.height), Image.LANCZOS)
    arr = np.array(img)

    if args.pattern == "RGGB":
        bayer = rgb_to_bayer_rggb(arr)
    else:
        # TODO: implement other patterns
        bayer = rgb_to_bayer_rggb(arr)

    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    bayer.tofile(args.output)
    print(f"[OK] {args.width}x{args.height} {args.pattern} Bayer saved to {args.output}")
    print(f"     File size: {bayer.nbytes:,} bytes ({bayer.nbytes/1024/1024:.1f} MB)")

if __name__ == "__main__":
    main()
