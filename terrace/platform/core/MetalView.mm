//
//  MetalView.mm
//  terrace
//
//  Created by celine on 2026-03-14.
//

#import "MetalView.h"

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
        app = new Application((__bridge MTL::Device*)self.device);
    }

    return self;
}

- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    // https://developer.apple.com/documentation/metalkit/mtkviewdelegate/mtkview(_:drawablesizewillchange:)
    return;
}


- (void)drawInMTKView:(MTKView *)view {
    if (!view.currentDrawable || !view.currentRenderPassDescriptor)
        return;
    app->update();
    
    app->render(
        (__bridge MTL::RenderPassDescriptor*)view.currentRenderPassDescriptor,
        (__bridge MTL::Drawable*)view.currentDrawable
    );
}
    
- (void)dealloc {
    std::cout << "debug";
    delete app;
}
 

@end
