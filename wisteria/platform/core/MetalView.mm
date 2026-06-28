//
//  MetalView.mm
//  wisteria
//
//  Created by celine on 2026-03-14.
//

#import "MetalView.h"
#include "platform/renderer/backend/RasterBackend.hpp"
#import <QuartzCore/QuartzCore.h>

@implementation MetalView {
    Application* app; // Pointer to your C++ engine
    CFTimeInterval _lastFrameTime;
}

// Implementation, this is what actually creates the view object, which we need to then insert into the view heiraarchy
// we override the device creation so that the view and device are connected at the start
- (instancetype) initWithFrame:(CGRect) frame device:(id<MTLDevice>)device {
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.delegate = self;
        self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        self.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
        self.framebufferOnly = NO;
        app = new Application((__bridge MTL::Device*)self.device);
        _lastFrameTime = CACurrentMediaTime();
    }

    return self;
}

-(void)applicationDidFinishLaunching:(NSNotification *)notification {
    return;
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    app->onResize(static_cast<uint32_t>(size.width),
                  static_cast<uint32_t>(size.height));
}

- (void)drawInMTKView:(MTKView *)view {
    if (!view.currentDrawable || !view.currentRenderPassDescriptor)
        return;
    app->update();

    CFTimeInterval now = CACurrentMediaTime();
    float dt = static_cast<float>(now - _lastFrameTime);
    _lastFrameTime = now;

    FrameContext ctx;
    ctx.renderPassDesc = (__bridge MTL::RenderPassDescriptor*)view.currentRenderPassDescriptor;
    ctx.drawable       = (__bridge MTL::Drawable*)view.currentDrawable;
    ctx.width          = static_cast<uint32_t>(view.drawableSize.width);
    ctx.height         = static_cast<uint32_t>(view.drawableSize.height);
    ctx.dt             = dt;

    app->render(ctx);
}

// mouse events

- (void) mouseDragged:(NSEvent *)event {
    app->onMouseDrag(event.deltaX, event.deltaY);
}

- (void) scrollWheel:(NSEvent *)event {
    app->onScroll(event.scrollingDeltaY);
}

- (void) keyUp:(NSEvent *)event {
    app->onKey(event.keyCode, false);
}

- (void) keyDown:(NSEvent *)event {
    app->onKey(event.keyCode, true);
}

- (BOOL)acceptsFirstResponder {
    return YES;
}
    
- (void)dealloc {
    std::cout << "debug";
    delete app;
}
 

@end
