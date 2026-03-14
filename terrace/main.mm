//
//  main.cpp
//  terrace
//
//  Created by celine on 2026-03-11.
//

#include <iostream>
#include <Metal/Metal.hpp>
#include "platform/ScopedReleasePool.hpp"
#include "platform/Application.hpp"
#import "platform/MetalView.h"

int main() {
    ScopedAutoreleasePool autoReleasePool;
        
    // 1. Get the GPU
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    
    if (!device) {
        std::cerr << "Metal is not supported on this device." << std::endl;
        return -1;
    }

    // 2. Print the name of your GPU (e.g., Apple M2 Max)
    std::cout << "Successfully initialized: " << device->name()->utf8String() << std::endl;

    // 3. Clean up (Metal-cpp uses manual reference counting)
    
    
    return 0;
}
