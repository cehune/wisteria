//
//  AssetPath.hpp
//  wisteria
//
//  Resolves paths to samples
//
//  repoRoot() walks up from this header's compile-time location (__FILE__)
//  until it finds the directory that contains "samples/", then assets are
//  resolved inside it. Replace with a real asset/bundle system once one exists.
//
#pragma once
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

namespace wisteria::assets {

// Directory containing the repo's "samples/" folder, found by walking up
// from this header's path. Returns an empty path if not found.
inline std::filesystem::path repoRoot() {
    namespace fs = std::filesystem;
    std::error_code ec;
    for (fs::path dir = fs::path(__FILE__).parent_path(); ; dir = dir.parent_path()) {
        if (fs::exists(dir / "samples", ec)) return dir;
        if (dir == dir.root_path()) return {};   // reached filesystem root
    }
}

// Absolute path to <root>/samples/<relative>. Honors the WISTERIA_SAMPLES
// environment variable as an override.
inline std::string samplePath(const std::string& relative) {
    namespace fs = std::filesystem;
    if (const char* env = std::getenv("WISTERIA_SAMPLES")) {
        return (fs::path(env) / relative).string();
    }
    return (repoRoot() / "samples" / relative).string();
}
}  // namespace wisteria::assets
