#import <Foundation/Foundation.h>

#import <mach-o/getsect.h>
#import <dlfcn.h>
#import <sys/stat.h>

#import "external/Metadata.h"
#import "external/SwiftDemangle.h"

#import "MachO.h"
#import "MetadataFuncs.h"
#import "MetadataDebug.h"

static NSString *demangleSwift(const char *const mangled) {
    size_t const length = swift_demangle_getDemangledName(mangled, NULL, 0);
    if (length == 0) {
        return nil;
    }
    char *const output = malloc(length + 1);
    swift_demangle_getDemangledName(mangled, output, length + 1);
    return [[NSString alloc] initWithBytesNoCopy:output length:length encoding:NSUTF8StringEncoding freeWhenDone:YES];
}

static inline BOOL stringHasPrefix(const char *const string, const char *prefix) {
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

static NSString *fullyQualifiedName(const struct TargetContextDescriptor *const desc) {
    const struct TargetContextDescriptor *const parent = relativeContextResolve(&desc->parent);
    NSString *const parentName = parent ? fullyQualifiedName(parent) : nil;
    NSString *localName = nil;
    
    switch (desc->flags.kind) {
        case ContextDescriptorKindModule: {
            const char *const name = relativeDirectResolve(&((const struct TargetModuleContextDescriptor *)desc)->name);
            localName = @(name);
        } break;
        case ContextDescriptorKindExtension: {
            // TODO
            assert(0 && "Extension in name resolution");
            return nil;
        } break;
        case ContextDescriptorKindAnonymous: {
            // TODO
            assert(0 && "Anonymous in name resolution");
            return nil;
        } break;
        case ContextDescriptorKindProtocol: {
            // TODO
            assert(0 && "Protocol in name resolution");
            return nil;
        } break;
        case ContextDescriptorKindOpaqueType:
            assert(0 && "OpaqueType in name resolution");
            return nil;
        case ContextDescriptorKindClass:
        case ContextDescriptorKindStruct:
        case ContextDescriptorKindEnum: {
            const char *const name = relativeDirectResolve(&((const struct TargetTypeContextDescriptor *)desc)->name);
            localName = @(name);
        } break;
        default:
            assert(0 && "Unknown descriptor kind in name resolution");
            return nil;
    }
    
    assert(localName != nil);
    if (parentName) {
        return [parentName stringByAppendingFormat:@".%@", localName];
    }
    return localName;
}

static NSString *resolveType(const struct TargetFieldRecord *const record) {
    const char *const potentialName = relativeDirectResolve(&record->mangledTypeName);
    const char symbolFlag = potentialName[0];
    const struct TargetContextDescriptor *reference = NULL;
    
    if (symbolFlag == 0) {
        assert(0 && "unexpected name (empty?)");
    } else if (symbolFlag <= 0x17) {
        switch (symbolFlag) {
            case 1: {
                reference = relativeDirectResolve((void *)&potentialName[1]);
            } break;
            case 2: {
                const struct TargetContextDescriptor *const *const referenceRef = relativeDirectResolve((void *)&potentialName[1]);
                // this is a linked symbol - only de-reference if this image is loaded with dyld
                Dl_info addr_info; // we don't use `addr_info`, but if we pass NULL, `dladdr` returns 0
                if (dladdr(referenceRef, &addr_info)) {
                    reference = *referenceRef;
                } else {
                    // TODO: Use reverseSymbolLookUp (cache the result)
                    // to get the symbol name. The symbol name should end with "Mn" ("nominal type descriptor").
                    // if it does, drop that suffix to get the underlying type. Demangle and return that.
                    assert(0 && "image must be loaded with dyld");
                }
            } break;
            default:
                assert(0 && "unsupported symbolic type");
                return nil;
        }
    } else if (symbolFlag <= 0x1f) {
        assert(0 && "[unsupported] absolute symbol reference");
        return nil;
    }
    
    if (reference) {
        return fullyQualifiedName(reference);
    }
    // https://github.com/swiftlang/swift/blob/586ac0953c15f775b93c8620b2b4433e111636ec/docs/ABI/Mangling.rst#mangling
    if (stringHasPrefix(potentialName, "$s")
        || stringHasPrefix(potentialName, "@__swiftmacro_")
        || stringHasPrefix(potentialName, "_T0")
        || stringHasPrefix(potentialName, "$S")) {
        return demangleSwift(potentialName);
    }
    // add the ABI stable mangling prefix
    size_t len = strlen(potentialName);
    size_t buffLen = 2 + len + 1;
    char *buff = malloc(buffLen);
    strncpy(buff, "$s", buffLen);
    strncpy(buff + 2, potentialName, buffLen - 2);
    NSString *result = demangleSwift(buff);
    free(buff);
    return result;
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        return 1;
    }
#if 1 /* currently we need to load with dyld (to get symbol fix-ups), hopefully this is temporary */
    void *handle = dlopen(argv[1], RTLD_NOW);
    assert(handle != NULL);
    
    struct stat target_stat;
    if (stat(argv[1], &target_stat) != 0) {
        assert(0 && "stat on target failed");
        return 1;
    }
    
    uint32_t count = _dyld_image_count();
    const void *mh = NULL;
    // go backwards because we just loaded the image, so it should be more recent (last)
    while (count > 0) {
        count--;
        
        struct stat test_stat;
        if (stat(_dyld_get_image_name(count), &test_stat) == 0) {
            if (test_stat.st_dev == target_stat.st_dev && test_stat.st_ino == target_stat.st_ino) {
                mh = _dyld_get_image_header(count);
                break;
            }
        }
    }
    assert(mh && "mach header not found");
#else
    NSError *error = nil;
    
    NSURL *target = [NSURL fileURLWithPath:@(argv[1])];
    NSData *data = [NSData dataWithContentsOfURL:target options:0 error:&error];
    if (data == nil) {
        NSLog(@"%@", error);
        return 1;
    }
    const void *const mh = machHeaderForImage(data.bytes);
#endif
    
    unsigned long sectSize = 0;
    const void *const sectHead = getsectiondata(mh, "__TEXT", "__swift5_types", &sectSize);
    if (sectHead == NULL) {
        assert(0 && "__TEXT,__swift5_types not found");
        return 1;
    }
    
    unsigned long const itemCount = sectSize / sizeof(int32_t);
    const int32_t *const items = sectHead;
    for (unsigned long index = 0; index < itemCount; index++) {
        int32_t const offset = items[index];
        const void *const base = items + index;
        
        const void *const item = base + offset;
        const struct TargetContextDescriptor *const casted = item;
        switch (casted->flags.kind) {
            case ContextDescriptorKindModule: {
                const struct TargetModuleContextDescriptor *const moduleDesc = (void *)casted;
                // TODO
                printf("/* Module not implemented */\n");
                (void)moduleDesc;
            } break;
            case ContextDescriptorKindExtension: {
                const struct TargetExtensionContextDescriptor *const extensionDesc = (void *)casted;
                // TODO
                printf("/* Extension not implemented */\n");
                (void)extensionDesc;
            } break;
            case ContextDescriptorKindAnonymous: {
                const struct TargetAnonymousContextDescriptor *const anonymousDesc = (void *)casted;
                // TODO
                printf("/* Anonymous not implemented */\n");
                (void)anonymousDesc;
            } break;
            case ContextDescriptorKindProtocol: {
                const struct TargetProtocolDescriptor *const protocolDesc = (void *)casted;
                // TODO
                printf("/* Protocol not implemented */\n");
                (void)protocolDesc;
            } break;
            case ContextDescriptorKindOpaqueType: {
                const struct TargetOpaqueTypeDescriptor *const opaqueDesc = (void *)casted;
                // TODO
                printf("/* OpaqueType not implemented */\n");
                (void)opaqueDesc;
            } break;
            case ContextDescriptorKindClass: {
                const struct TargetClassDescriptor *const classDesc = (void *)casted;
                const struct TargetFieldDescriptor *const fieldsDesc = relativeDirectResolve(&classDesc->typeContext.fields);
                
                printf("class %s {\n", [fullyQualifiedName(casted) UTF8String]);
                if (fieldsDesc != NULL) {
                    const struct TargetFieldRecord *const fields = (const void *)(fieldsDesc + 1);
                    for (uint32_t i = 0; i < fieldsDesc->numFields; i++) {
                        const struct TargetFieldRecord *const field = fields + i;
                        
                        NSString *name = resolveType(field);
                        
                        printf("\t%s %s: %s // flags: %" __UINT32_FMTu__ "\n",
                               (field->flags & TargetFieldRecordFlagsIsVar) ? "var" : "let",
                               (const char *)relativeDirectResolve(&field->fieldName),
                               [name UTF8String],
                               field->flags);
                    }
                }
                printf("}\n");
            } break;
            case ContextDescriptorKindStruct: {
                const struct TargetStructDescriptor *const structDesc = (void *)casted;
                const struct TargetFieldDescriptor *const fieldsDesc = relativeDirectResolve(&structDesc->valueType.typeContext.fields);
                
                printf("struct %s {\n", [fullyQualifiedName(casted) UTF8String]);
                if (fieldsDesc != NULL) {
                    const struct TargetFieldRecord *const fields = (const void *)(fieldsDesc + 1);
                    for (uint32_t i = 0; i < fieldsDesc->numFields; i++) {
                        const struct TargetFieldRecord *const field = fields + i;
                        
                        NSString *name = resolveType(field);
                        
                        printf("\t%s %s: %s // flags: %" __UINT32_FMTu__ "\n",
                               (field->flags & TargetFieldRecordFlagsIsVar) ? "var" : "let",
                               (const char *)relativeDirectResolve(&field->fieldName),
                               [name UTF8String],
                               field->flags);
                    }
                }
                printf("}\n");
            } break;
            case ContextDescriptorKindEnum: {
                const struct TargetEnumDescriptor *const enumDesc = (void *)casted;
                const struct TargetFieldDescriptor *const fieldsDesc = relativeDirectResolve(&enumDesc->valueType.typeContext.fields);
                
                printf("enum %s {\n", [fullyQualifiedName(casted) UTF8String]);
                if (fieldsDesc != NULL) {
                    const struct TargetFieldRecord *const fields = (const void *)(fieldsDesc + 1);
                    for (uint32_t i = 0; i < fieldsDesc->numFields; i++) {
                        const struct TargetFieldRecord *const field = fields + i;
                        
                        printf("\tcase %s // flags: %" __UINT32_FMTu__ "\n",
                               (const char *)relativeDirectResolve(&field->fieldName),
                               field->flags);
                    }
                }
                printf("}\n");
            } break;
        }
    }
}
