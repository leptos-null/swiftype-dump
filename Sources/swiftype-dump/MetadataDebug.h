#ifndef MetadataDebug_h
#define MetadataDebug_h

#import "external/Metadata.h"

/* functions to print (for debugging) the structs in Metadata */

#if DEBUG

void debugPutTargetFieldDescriptor(const struct TargetFieldDescriptor *const desc);

void debugPutTargetClassDescriptor(const struct TargetClassDescriptor *const desc);
void debugPutTargetStructDescriptor(const struct TargetStructDescriptor *const desc);
void debugPutTargetEnumDescriptor(const struct TargetEnumDescriptor *const desc);

#endif /* DEBUG */

#endif /* MetadataDebug_h */
