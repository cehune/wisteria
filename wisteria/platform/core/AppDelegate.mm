//
//  AppDelegate.mm
//  wisteria
//
//  Created by celine on 2026-03-14.
//
#import "AppDelegate.h"

@implementation AppDelegate {
    NSWindow* window;
}

-(void)applicationDidFinishLaunching:(NSNotification*)notification {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    
    CGRect frame = CGRectMake(0,0,800,600);
    
    window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(
                   NSWindowStyleMaskTitled |
                   NSWindowStyleMaskClosable |
                   NSWindowStyleMaskResizable
                   ) 
        backing:NSBackingStoreBuffered
              defer:NO];
    MetalView* view = [[MetalView alloc] initWithFrame:frame device:device];
    
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
