#!/usr/bin/env python3
"""
visualize.py
Reads a .raw Bayer file and displays simulated pipeline stage outputs
side-by-side using matplotlib.

Usage:
    python sim/visualize.py --input input/test_frame.raw
"""
import argparse
import numpy as np
from pathlib import Path

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input",  required=True)
    parser.add_argument("--width",  type=int, default=1920)
    parser.add_argument("--height", type=int, default=1080)
    args = parser.parse_args()

    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("Install matplotlib: pip install matplotlib"); return

    raw = np.fromfile(args.input, dtype=np.uint16)
    raw = raw.reshape((args.height, args.width))

    # Show Bayer mosaic (scaled)
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    axes[0].imshow(raw >> 2, cmap="gray", vmin=0, vmax=255)
    axes[0].set_title("Raw Bayer (green channel dominant)")

    # Simple debayer for visualisation (not the C++ pipeline)
    rgb = np.stack([raw[0::2, 0::2], raw[0::2, 1::2], raw[1::2, 1::2]], axis=-1)
    rgb_small = (rgb >> 2).astype(np.uint8)
    axes[1].imshow(rgb_small)
    axes[1].set_title("After demosaic (Python preview)")

    # Histogram
    axes[2].hist(raw.flatten() >> 2, bins=256, range=(0,255), color="green", alpha=0.7)
    axes[2].set_title("Pixel intensity histogram")

    plt.tight_layout()
    plt.savefig("stage_preview.png", dpi=100)
    print("[OK] Saved stage_preview.png")
    plt.show()

if __name__ == "__main__":
    main()
