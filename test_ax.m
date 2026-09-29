#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        for (int i=0; i<10; i++) {
            CGEventRef event = CGEventCreate(NULL);
            CGPoint mouseLoc = CGEventGetLocation(event);
            CFRelease(event);
            
            AXUIElementRef systemWideElement = AXUIElementCreateSystemWide();
            AXUIElementRef elementAtPosition = NULL;
            AXError err = AXUIElementCopyElementAtPosition(systemWideElement, mouseLoc.x, mouseLoc.y, &elementAtPosition);
            
            if (err == kAXErrorSuccess && elementAtPosition) {
                CFTypeRef role;
                if (AXUIElementCopyAttributeValue(elementAtPosition, kAXRoleAttribute, &role) == kAXErrorSuccess) {
                    NSLog(@"Mouse at (%.0f, %.0f), Element Role: %@", mouseLoc.x, mouseLoc.y, role);
                    CFRelease(role);
                }
                CFRelease(elementAtPosition);
            } else {
                NSLog(@"Mouse at (%.0f, %.0f), No element found", mouseLoc.x, mouseLoc.y);
            }
            
            CFRelease(systemWideElement);
            sleep(1);
        }
    }
    return 0;
}
