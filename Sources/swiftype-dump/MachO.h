#ifndef MachO_h
#define MachO_h

#import <Foundation/Foundation.h>

#import <mach-o/dyld.h>
#import <mach-o/fat.h>
#import <mach-o/nlist.h>

/// key is an address in `mh` where data could be read from,
/// value is the symbol name the data is bound to
NSDictionary<NSValue *, NSString *> *reverseSymbolLookUp(const struct mach_header *const mh);

const struct mach_header *machHeaderForImage(const void *const image);

#endif /* MachO_h */
