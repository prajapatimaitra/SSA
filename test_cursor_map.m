#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <CommonCrypto/CommonDigest.h>

NSString * md5(NSData * data) {
    unsigned char result[CC_MD5_DIGEST_LENGTH];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CC_MD5(data.bytes, (CC_LONG)data.length, result);
#pragma clang diagnostic pop
    return [NSString stringWithFormat:
            @"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            result[0], result[1], result[2], result[3], 
            result[4], result[5], result[6], result[7],
            result[8], result[9], result[10], result[11],
            result[12], result[13], result[14], result[15]
            ];
}

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        NSDictionary *cursors = @{
            @"arrow": [NSCursor arrowCursor],
            @"iBeam": [NSCursor IBeamCursor],
            @"pointingHand": [NSCursor pointingHandCursor],
            @"closedHand": [NSCursor closedHandCursor],
            @"openHand": [NSCursor openHandCursor],
            @"resizeLeft": [NSCursor resizeLeftCursor],
            @"resizeRight": [NSCursor resizeRightCursor],
            @"resizeLeftRight": [NSCursor resizeLeftRightCursor],
            @"resizeUp": [NSCursor resizeUpCursor],
            @"resizeDown": [NSCursor resizeDownCursor],
            @"resizeUpDown": [NSCursor resizeUpDownCursor],
            @"crosshair": [NSCursor crosshairCursor],
            @"disappearingItem": [NSCursor disappearingItemCursor],
            @"operationNotAllowed": [NSCursor operationNotAllowedCursor],
            @"dragLink": [NSCursor dragLinkCursor],
            @"dragCopy": [NSCursor dragCopyCursor],
            @"contextualMenu": [NSCursor contextualMenuCursor],
            @"iBeamCursorForVerticalLayout": [NSCursor IBeamCursorForVerticalLayout]
        };
        
        for (NSString *key in cursors) {
            NSCursor *c = cursors[key];
            NSImage *img = [c image];
            NSData *tiff = [img TIFFRepresentation];
            NSString *hash = md5(tiff);
            NSLog(@"{\"%@\", \"%@\"}, // size: %@", hash, key, NSStringFromSize(img.size));
        }
    }
    return 0;
}
