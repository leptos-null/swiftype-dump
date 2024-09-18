#ifndef MetadataFuncs_h
#define MetadataFuncs_h

#import <stdbool.h>
#import "external/Metadata.h"

static inline const void *_Nullable relativeDirectResolve(const TargetRelativeDirectPointer *_Nonnull const value) {
    const void *const base = value;
    int32_t const offset = *value;
    if (offset == 0) {
        return NULL;
    }
    return base + offset;
}

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/Basic/RelativePointer.h#L277
static inline const void *_Nullable relativeContextResolve(const TargetRelativeContextPointer *_Nonnull const value) {
    const void *const base = value;
    int32_t const offset = *value;
    if (offset == 0) {
        return NULL;
    }
    // indirect
    if (offset & 1) {
        const void *const *const resolved = base + (offset & ~1);
        return *resolved;
    }
    return base + offset;
}

#endif /* MetadataFuncs_h */
