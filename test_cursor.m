#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <CommonCrypto/CommonDigest.h>
#include <unistd.h>

NSString * md5(NSData * data) {
    unsigned char result[CC_MD5_DIGEST_LENGTH];
    CC_MD5(data.bytes, (CC_LONG)data.length, result);
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
        for (int i=0; i<15; i++) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            NSCursor *cursor = [NSCursor currentSystemCursor];
#pragma clang diagnostic pop
            
            NSImage *image = [cursor image];
            NSData *tiffData = [image TIFFRepresentation];
            NSString *hash = md5(tiffData);
            
            NSLog(@"Cursor Hash: %@, Size: %@, Hotspot: %@", hash, NSStringFromSize(image.size), NSStringFromPoint([cursor hotSpot]));
            sleep(1);
        }
    }
    return 0;
}
