//
//  MetalView.mm
//  terrace
//
//  Created by celine on 2026-03-14.
//

#import "MetalView.h"
#include "platform/renderer/backend/RasterBackend.hpp"

@implementation MetalView {
    Application* app; // Pointer to your C++ engine
}

// Implementation, this is what actually creates the view object, which we need to then insert into the view heiraarchy
// we override the device creation so that the view and device are connected at the start
- (instancetype) initWithFrame:(CGRect) frame device:(id<MTLDevice>)device {
    self = [super initWithFrame:frame device:device];
    if (self) {
        self.delegate = self;
        self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        self.framebufferOnly = NO;  
        app = new Application((__bridge MTL::Device*)self.device);
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
    
    FrameContext ctx;
    ctx.renderPassDesc = (__bridge MTL::RenderPassDescriptor*)view.currentRenderPassDescriptor;
    ctx.drawable       = (__bridge MTL::Drawable*)view.currentDrawable;
    ctx.width          = static_cast<uint32_t>(view.drawableSize.width);
    ctx.height         = static_cast<uint32_t>(view.drawableSize.height);

    app->render(ctx);
}
    
- (void)dealloc {
    std::cout << "debug";
    delete app;
}
 

@end
