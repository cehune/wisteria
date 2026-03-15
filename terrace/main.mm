//
//  main.cpp
//  terrace
//
//  Created by celine on 2026-03-11.
//






#import <AppKit/AppKit.h>
#import "platform/core/AppDelegate.h"
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        AppDelegate* delegate = [[AppDelegate alloc] init ];
        
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}
