#include "MetadataDebug.h"
#include "MetadataFuncs.h"

#if DEBUG

#include <stdio.h>

void debugPutTargetFieldDescriptor(const struct TargetFieldDescriptor *const desc) {
    printf("(%p) struct TargetFieldDescriptor {\n"
           "\tmangledTypeName: %s\n"
           "\tsuperclass: %p\n"
           "\tkind: %" __UINT16_FMTu__ "\n"
           "\tfieldRecordSize: %" __UINT16_FMTu__ "\n"
           "\tnumFields: %" __UINT32_FMTu__ "\n"
           "}\n",
           desc,
           (const char *)relativeDirectResolve(&desc->mangledTypeName),
           relativeDirectResolve(&desc->superclass),
           desc->kind,
           desc->fieldRecordSize,
           desc->numFields
           );
    
    const void *const fields = (const void *)(desc + 1);
    for (uint32_t i = 0; i < desc->numFields; i++) {
        // byte arithmetic
        const struct TargetFieldRecord *const field = fields + (i * desc->fieldRecordSize);
        
        printf("(%p) struct TargetFieldRecord {\n"
               "\tflags: %" __UINT32_FMTu__ "\n"
               "\tmangledTypeName: %s\n"
               "\tfieldName: %s\n"
               "}\n",
               field,
               field->flags,
               (const char *)relativeDirectResolve(&field->mangledTypeName),
               (const char *)relativeDirectResolve(&field->fieldName)
               );
    }
}

void debugPutTargetClassDescriptor(const struct TargetClassDescriptor *const desc) {
    printf("(%p) struct TargetClassDescriptor {\n"
           "\tname: %s\n"
           "\tnumFields: %" __UINT32_FMTu__ "\n"
           "\tfieldOffsetVectorOffset: %" __UINT32_FMTu__ "\n"
           "}\n",
           desc,
           (const char *)relativeDirectResolve(&desc->typeContext.name),
           desc->numFields,
           desc->fieldOffsetVectorOffset
           );
    debugPutTargetFieldDescriptor(relativeDirectResolve(&desc->typeContext.fields));
}

void debugPutTargetStructDescriptor(const struct TargetStructDescriptor *const desc) {
    printf("(%p) struct TargetStructDescriptor {\n"
           "\tname: %s\n"
           "\tnumFields: %" __UINT32_FMTu__ "\n"
           "\tfieldOffsetVectorOffset: %" __UINT32_FMTu__ "\n"
           "}\n",
           desc,
           (const char *)relativeDirectResolve(&desc->valueType.typeContext.name),
           desc->numFields,
           desc->fieldOffsetVectorOffset
           );
    debugPutTargetFieldDescriptor(relativeDirectResolve(&desc->valueType.typeContext.fields));
}

void debugPutTargetEnumDescriptor(const struct TargetEnumDescriptor *const desc) {
    printf("(%p) struct TargetEnumDescriptor {\n"
           "\tname: %s\n"
           "\tnumPayloadCases: %" __UINT32_FMTu__ "\n"
           "\tpayloadSizeOffset: %" __UINT32_FMTu__ "\n"
           "\tnumEmptyCases: %" __UINT32_FMTu__ "\n"
           "}\n",
           desc,
           (const char *)relativeDirectResolve(&desc->valueType.typeContext.name),
           desc->numPayloadCasesAndPayloadSizeOffset.numPayloadCases,
           desc->numPayloadCasesAndPayloadSizeOffset.payloadSizeOffset,
           desc->numEmptyCases
           );
    debugPutTargetFieldDescriptor(relativeDirectResolve(&desc->valueType.typeContext.fields));
}

#endif
