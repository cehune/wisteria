//
//  AppDelegate.h
//  terrace
//
//  Created by celine on 2026-03-14.
//
#pragma once
#import <MetalKit/MetalKit.h>
#import "MetalView.h"
#import <Metal/Metal.h>

@interface AppDelegate: NSObject <NSApplicationDelegate, NSWindowDelegate>
-(void)applicationDidFinishLaunching:(NSNotification*)notification;
@end
