#include <AppKit/AppKit.h>

extern "C" void mayoInitCocoa()
{
    NSApplicationLoad();
    if (NSApp == nil) {
        [NSApplication sharedApplication];
    }
}
