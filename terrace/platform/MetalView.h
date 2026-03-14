//
//  MetalView.h
//  terrace
//
//  Created by celine on 2026-03-14.
//

//We require a bridge for our c++ to actual be aware of the windows. Objective C will act as the bridge
//so we can use native Cocoa functionality

#import <MetalKit/MetalKit.h>

@interface MetalView: MTKView<MTKViewDelegate>

@end
