//
//  AppDelegate.mm
//  wisteria
//
//  Created by celine on 2026-03-14.
//
#import "AppDelegate.h"

@implementation AppDelegate {
    NSWindow*    window;
    RenderConfig _config;
}

- (instancetype)initWithConfig:(const RenderConfig&)config {
    self = [super init];
    if (self) {
        _config = config;
    }
    return self;
}

-(void)applicationDidFinishLaunching:(NSNotification*)notification {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    // Window size comes from the config (also the drawable is 2x on retina and
    // the backend adopts that pixel size per-frame bc it reads framecontext width/height)
    CGRect frame = CGRectMake(0, 0, _config.width, _config.height);

    window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(
                   NSWindowStyleMaskTitled |
                   NSWindowStyleMaskClosable |
                   NSWindowStyleMaskResizable
                   )
        backing:NSBackingStoreBuffered
              defer:NO];
    MetalView* view = [[MetalView alloc] initWithFrame:frame device:device config:_config];

    [window setContentView:view];
    [window setDelegate:self];
    // This ones required for key events to MetalView, ie camera controls, etc
    [window makeFirstResponder:view];
    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];   // pull app to foreground so it gets key focus
}

- (void) windowWillClose:(NSNotification *)notification {
    [NSApp terminate:nil];
}

@end
