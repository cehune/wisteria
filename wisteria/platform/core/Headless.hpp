//
//  Headless.hpp
//  wisteria
//
//  Created by celine on 2026-07-28.
//  Render and get an image output without launching the app window (from)
//  TODO: maybe make the output configure to the scene dimensions if known
//  especially if the input is mitsuba style
//
//    ./build/Debug/wisteria --headless --spp 512 --width 800 --height 600 --out cornell.pfm
//
#pragma once
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include "platform/core/Application.hpp"
#include "platform/renderer/RenderConfig.hpp"

#include <exception>
#include <iostream>

inline int runOffline(const RenderConfig& config) {
    // AppKit would normally supply this
    NS::AutoreleasePool* arp = NS::AutoreleasePool::alloc()->init();

    if (config.width == 0 || config.height == 0) {
        std::cerr << "offline: --width/--height must be non-zero\n";
        arp->release();
        return 1;
    }

    // Works with no window and no NSApplication.
    MTL::Device* dev = MTL::CreateSystemDefaultDevice();
    if (!dev) {
        std::cerr << "offline: no Metal device available\n";
        arp->release();
        return 1;
    }

    int rc = 0;
    {
        // Make an application to run the code. 
        // Any error is more likely cli mistake
        try {
            Application app(dev, config);
            if (!app.renderOffline()) rc = 1;
        } catch (const std::exception& e) {
            std::cerr << "offline: "
                      << (config.scenePath.empty() ? "cornell_box.obj (default)"
                                                   : config.scenePath)
                      << ": " << e.what() << "\n";
            rc = 1;
        }
    }   // Application torn down before the device is released

    dev->release();
    arp->release();
    return rc;
}
