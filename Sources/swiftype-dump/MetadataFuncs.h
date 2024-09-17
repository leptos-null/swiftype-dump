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
