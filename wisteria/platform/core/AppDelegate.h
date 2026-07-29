//
//  AppDelegate.h
//  wisteria
//
//  Created by celine on 2026-03-14.
//
#pragma once
#import <MetalKit/MetalKit.h>
#import "MetalView.h"
#import <Metal/Metal.h>
#include "platform/renderer/RenderConfig.hpp"

@interface AppDelegate: NSObject <NSApplicationDelegate, NSWindowDelegate>
- (instancetype)initWithConfig:(const RenderConfig&)config;
-(void)applicationDidFinishLaunching:(NSNotification*)notification;
@end
