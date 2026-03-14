//
//  ScopedReleasePool.hpp
//  terrace
//
//  Created by celine on 2026-03-12.
//
#pragma once
#include <Metal/Metal.hpp>

class ScopedAutoreleasePool {
public:
    
    ScopedAutoreleasePool():
    _pool(NS::AutoreleasePool::alloc()->init()) {}
    ~ScopedAutoreleasePool() {
        _pool->release();
    }
    
    // Disable copying
    ScopedAutoreleasePool(const ScopedAutoreleasePool&) = delete;
    ScopedAutoreleasePool& operator=(const ScopedAutoreleasePool&) = delete;

private:
    NS::AutoreleasePool* _pool;
};
