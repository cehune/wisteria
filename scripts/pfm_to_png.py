#!/usr/bin/env python3
import sys
from pathlib import Path
import numpy as np
from PIL import Image


def read_pfm(file_path: Path) -> np.ndarray:
    """Reads a color (PF) or grayscale (Pf) PFM image into a float32 numpy array."""
    with open(file_path, "rb") as f:
        # Read magic header
        header = f.readline().decode("ascii").strip()
        if header not in ("PF", "Pf"):
            raise ValueError(f"Invalid PFM header: {header}")

        channels = 3 if header == "PF" else 1

        # Read dimensions (skip any comment lines)
        dims_line = f.readline().decode("ascii").strip()
        while dims_line.startswith("#"):
            dims_line = f.readline().decode("ascii").strip()
        width, height = map(int, dims_line.split())

        # Read scale factor (negative value indicates little-endian)
        scale = float(f.readline().decode("ascii").strip())
        endian = "<" if scale < 0 else ">"

        # Read binary floating point pixel data
        data = np.frombuffer(f.read(), dtype=f"{endian}f")
        if data.size != width * height * channels:
            raise ValueError("Pixel data size does not match header metadata.")

        # Reshape array
        if channels == 3:
            image = data.reshape((height, width, 3))
        else:
            image = data.reshape((height, width))

        # PFM stores rows bottom-to-top; flip vertically for PNG/standard display
        return np.flipud(image)


def main():
    if len(sys.argv) < 2:
        print("Usage: python scripts/pfm_to_png.py <path_to_pfm>")
        sys.exit(1)

    pfm_path = Path(sys.argv[1]).resolve()
    if not pfm_path.is_file():
        print(f"Error: File '{pfm_path}' not found.")
        sys.exit(1)

    # Output directory relative to script: repo_root/scripts/outputs
    script_dir = Path(__file__).resolve().parent
    output_dir = script_dir / "outputs/pfm_to_png/"
    output_dir.mkdir(parents=True, exist_ok=True)

    output_png = output_dir / f"{pfm_path.stem}.png"

    print(f"Reading PFM: {pfm_path.name}")
    hdr_data = read_pfm(pfm_path)

    # 1. Clamp negative values caused by Monte Carlo sample noise
    hdr_data = np.maximum(hdr_data, 0.0)

    # 2. Reinhard tonemapping (compresses HDR values > 1.0 down into printable range)
    tonemapped = hdr_data / (1.0 + hdr_data)

    # 3. Gamma 2.2 correction (Linear space -> sRGB display space)
    srgb = np.power(tonemapped, 1.0 / 2.2)

    # 4. Scale to 8-bit uint8 [0, 255]
    ldr_data = (srgb * 255.0).clip(0, 255).astype(np.uint8)

    # Save PNG
    img = Image.fromarray(ldr_data)
    img.save(output_png)

    print(f"Saved PNG to: {output_png}")


if __name__ == "__main__":
    main()