//
//  main.mm
//  wisteria
//
//  Created by celine on 2026-03-11.
//

#import <AppKit/AppKit.h>
#import "platform/core/AppDelegate.h"
#include "platform/renderer/RenderConfig.hpp"
#include "platform/core/Headless.hpp"
#include <cstdlib>
#include <cstring>
#include <string>

// Args (both modes): --scene PATH | --width N | --height N | --backend raster|pt
// | --headless | --spp N (sample target) | --out PATH
static RenderConfig parseRenderConfig(int argc, const char* argv[]) {
    RenderConfig cfg;
    bool sawBackend = false;
    for (int i = 1; i < argc; ++i) {
        auto uintArg = [&](uint32_t& dst) {
            if (i + 1 < argc) { dst = static_cast<uint32_t>(std::strtoul(argv[i + 1], nullptr, 10)); ++i; }
        };
        if      (std::strcmp(argv[i], "--width")   == 0) uintArg(cfg.width);
        else if (std::strcmp(argv[i], "--height")  == 0) uintArg(cfg.height);
        else if (std::strcmp(argv[i], "--spp")     == 0) uintArg(cfg.targetSamples);
        else if (std::strcmp(argv[i], "--headless") == 0) cfg.headless = true;
        else if (std::strcmp(argv[i], "--scene")   == 0 && i + 1 < argc) cfg.scenePath = argv[++i];
        else if (std::strcmp(argv[i], "--out")     == 0 && i + 1 < argc) cfg.outPath = argv[++i];
        else if (std::strcmp(argv[i], "--backend") == 0 && i + 1 < argc) {
            std::string b = argv[++i];
            cfg.backend = (b == "pt" || b == "pathtracer") ? BackendType::PathTracer
                                                           : BackendType::Raster;
            sawBackend = true;
        }
    }

    // just produces an image from cmd line
    // TODO: image output should print full path
    if (cfg.headless && !sawBackend) cfg.backend = BackendType::PathTracer;

    return cfg;
}

int main(int argc, const char* argv[]) {
    RenderConfig config = parseRenderConfig(argc, argv);

    // Two drivers over one Application: the offline loop, or the window.
    if (config.headless) return runOffline(config);

    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        // Become a regular foreground app so the window can become key and
        // receive keyDown/keyUp.
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        AppDelegate* delegate = [[AppDelegate alloc] initWithConfig:config];

        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}
