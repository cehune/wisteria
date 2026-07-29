//
//  AssetPath.hpp
//  wisteria
//
//  Created by celine on 2026-07-28.
//  Resolves paths to outputs
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

// Absolute path to <root>/outputs/<subdir>/<relative>, creating the directory
// if it doesn't exist.
inline std::string outputPath(const std::string& relative,
                              const std::string& subdir = "") {
    namespace fs = std::filesystem;
    fs::path dir;
    if (const char* env = std::getenv("WISTERIA_OUTPUTS")) {
        dir = fs::path(env);
    } else {
        fs::path root = repoRoot();
        if (root.empty()) return {};
        dir = root / "outputs";
    }
    if (!subdir.empty()) dir /= subdir;

    std::error_code ec;
    fs::create_directories(dir, ec);   // no-op if it already exists
    if (ec) return {};

    return (dir / relative).string();
}

// Where a render should actually be written
//
//   requested empty    -> <outputs>/<ext>/<defaultName>
//   requested relative -> <outputs>/<ext>/<requested>
//   requested absolute -> exactly as given, an explicit instruction
inline std::filesystem::path resolveOutputPath(const std::string& requested,
                                               const std::string& defaultName,
                                               const std::string& extension) {
    namespace fs = std::filesystem;
    const std::string want = requested.empty() ? defaultName : requested;

    fs::path result;
    if (fs::path(want).is_absolute()) {
        result = want;
    } else {
        // append extension (pfm default) not replace in case their name is like blah.1.5
        const std::string subdir = extension.empty() || extension[0] != '.'
            ? extension : extension.substr(1);
        const std::string resolved = outputPath(want, subdir);
        result = resolved.empty() ? fs::path(want) : fs::path(resolved);
    }

    if (result.extension() != extension) result += extension;
    return result;
}

// Absolute, symlink-resolved form, for logging. A relative name in a message
// is meaningless without knowing where the binary was launched from.
inline std::filesystem::path displayPath(const std::filesystem::path& p) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path r = fs::weakly_canonical(p, ec);
    if (ec) r = fs::absolute(p, ec);
    return ec ? p : r;
}
}  // namespace wisteria::assets
