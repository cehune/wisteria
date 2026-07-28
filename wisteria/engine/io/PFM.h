//
//  PFM.h
//  wisteria
//
//  Minimal Portable FloatMap (PFM) writer: linear HDR, lossless, zero deps.
//  Input is the RGBA32F accumulation readback (4 floats/pixel, row 0 = TOP, the
//  Metal texture order). We emit RGB only -- the alpha channel is the per-pixel
//  sample count N, not color -- and flip to PFM's bottom-to-top row order.
//
//  Format (three ASCII header lines, then raw floats):
//    "PF"      3-channel colour  ("Pf" would be grayscale)
//    "W H"
//    "-1.0"    scale/endianness: NEGATIVE => little-endian raster (Apple Silicon,
//              x86). A positive value claims big-endian and readers byte-swap.
//

#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace wisteria::pfm {

// rgba: width*height*4 floats, row-major, row 0 = TOP. Writes a little-endian
// "PF" colour PFM. Returns false on bad args or any I/O failure.
inline bool writePFM(const std::string& path, const float* rgba,
                     uint32_t width, uint32_t height) {
    if (!rgba || width == 0 || height == 0) return false;

    std::FILE* fp = std::fopen(path.c_str(), "wb");
    if (!fp) return false;

    std::fprintf(fp, "PF\n%u %u\n-1.0\n", width, height);

    // Pack RGB (drop A) into a contiguous buffer, flipped bottom-to-top, then one
    // write. Native float is already little-endian here, matching the header.
    std::vector<float> rgb(static_cast<size_t>(width) * height * 3);
    for (uint32_t y = 0; y < height; ++y) {
        const float* src = rgba + static_cast<size_t>(height - 1u - y) * width * 4u; // flip
        float*       dst = rgb.data() + static_cast<size_t>(y) * width * 3u;
        for (uint32_t x = 0; x < width; ++x) {
            dst[x * 3 + 0] = src[x * 4 + 0];   // R
            dst[x * 3 + 1] = src[x * 4 + 1];   // G
            dst[x * 3 + 2] = src[x * 4 + 2];   // B  (src[x*4+3] = N, dropped)
        }
    }

    const size_t count = static_cast<size_t>(width) * height * 3;
    const bool   ok    = std::fwrite(rgb.data(), sizeof(float), count, fp) == count;

    std::fclose(fp);
    return ok;
}

} // namespace wisteria::pfm
