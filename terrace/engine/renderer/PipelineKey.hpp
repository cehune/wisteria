//
//  PipelineKey.hpp
//  terrace
//
//  Created by celine on 2026-03-21.
//
#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <cstdint>

enum class PixelFormat {
    BGRA8Unorm,
    RGBA16Float
};

struct PipelineKey {
    uint32_t vertID;
    uint32_t fragID;
    PixelFormat pixelFormat = PixelFormat::BGRA8Unorm;

    bool operator==(const PipelineKey& other) const {
        return vertID == other.vertID &&
               fragID == other.fragID &&
               pixelFormat == other.pixelFormat;
    }
};

template<>
struct std::hash<PipelineKey> {
    size_t operator()(const PipelineKey& key) const {
        // Using a simple but effective pointer-mixing hash
        size_t h = key.vertID;
        h ^= key.fragID + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= static_cast<size_t>(key.pixelFormat) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

