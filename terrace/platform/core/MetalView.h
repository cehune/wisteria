//
//  MetalView.h
//  terrace
//
//  Created by celine on 2026-03-14.
//

//We require a bridge for our c++ to actual be aware of the windows. Objective C will act as the bridge
//so we can use native Cocoa functionality
#pragma once
#import <MetalKit/MetalKit.h>
#include "Application.hpp"

@interface MetalView: MTKView<MTKViewDelegate>
- (instancetype) initWithFrame:(CGRect) frame device:(id<MTLDevice>)device;
- (void)applicationDidFinishLaunching:(NSNotification*)notification;
- (void)drawInMTKView:(MTKView *)view;
@end
