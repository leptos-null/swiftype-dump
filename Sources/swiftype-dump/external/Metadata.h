// based on https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h

#import <stdint.h>
#import <stdbool.h>

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/MetadataValues.h#L1511
typedef enum : uint8_t {
    /// This context descriptor represents a module.
    ContextDescriptorKindModule,
    /// This context descriptor represents an extension.
    ContextDescriptorKindExtension,
    /// This context descriptor represents an anonymous possibly-generic context
    /// such as a function body.
    ContextDescriptorKindAnonymous,
    /// This context descriptor represents a protocol context.
    ContextDescriptorKindProtocol,
    /// This context descriptor represents an opaque type alias.
    ContextDescriptorKindOpaqueType,
    
    /// This context descriptor represents a class.
    ContextDescriptorKindClass = 16,
    /// This context descriptor represents a struct.
    ContextDescriptorKindStruct,
    /// This context descriptor represents an enum.
    ContextDescriptorKindEnum,
} ContextDescriptorKind;

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/Basic/RelativePointer.h#L232
/// A relative reference to an object stored in memory. The reference may be
/// direct or indirect, and uses the low bit of the (assumed at least
/// 2-byte-aligned) pointer to differentiate.
typedef int32_t TargetRelativeContextPointer;
typedef int32_t TargetRelativeDirectPointer;


// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/MetadataValues.h#L1545
struct __attribute__((__packed__)) ContextDescriptorFlags {
    /// The kind of context this descriptor describes.
    ContextDescriptorKind kind : 5;
    uint8_t _padding : 1; // unused as far as I know
    /// Whether this is a unique record describing the referenced context.
    bool isUnique : 1;
    /// Whether the context being described is generic.
    bool isGeneric : 1;
    /// The format version of the descriptor. Higher version numbers may have
    /// additional fields that aren't present in older versions.
    uint8_t version;
    /// The most significant two bytes of the flags word, which can have
    /// kind-specific meaning.
    uint16_t kindSpecificFlags;
};
_Static_assert(sizeof(struct ContextDescriptorFlags) == sizeof(uint32_t), "ContextDescriptorFlags is backed by a uint32_t");

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L2832
struct TargetContextDescriptor {
    /// Flags describing the context, including its kind and format version.
    struct ContextDescriptorFlags flags;
    /// The parent context, or null if this is a top-level context.
    TargetRelativeContextPointer parent;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L2890
struct TargetModuleContextDescriptor {
    struct TargetContextDescriptor context;
    
    /// The module name.
    TargetRelativeDirectPointer name;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L2932
struct TargetExtensionContextDescriptor {
    struct TargetContextDescriptor context;
    
    /// A mangling of the `Self` type context that the extension extends.
    /// The mangled name represents the type in the generic context encoded by
    /// this descriptor. For example, a nongeneric nominal type extension will
    /// encode the nominal type name. A generic nominal type extension will encode
    /// the instance of the type with any generic arguments bound.
    ///
    /// Note that the Parent of the extension will be the module context the
    /// extension is declared inside.
    TargetRelativeDirectPointer extendedContext;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L2972
struct TargetAnonymousContextDescriptor {
    struct TargetContextDescriptor context;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L3054
struct TargetProtocolDescriptor {
    struct TargetContextDescriptor context;
    
    /// The name of the protocol.
    TargetRelativeDirectPointer name;
    /// The number of generic requirements in the requirement signature of the
    /// protocol.
    uint32_t numRequirementsInSignature;
    /// The number of requirements in the protocol.
    /// If any requirements beyond MinimumWitnessTableSizeInWords are present
    /// in the witness table template, they will be not be overwritten with
    /// defaults.
    uint32_t numRequirements;
    /// Associated type names, as a space-separated list in the same order
    /// as the requirements.
    TargetRelativeDirectPointer associatedTypeNames;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L3146
struct TargetOpaqueTypeDescriptor {
    struct TargetContextDescriptor context;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L3724
struct TargetTypeContextDescriptor {
    struct TargetContextDescriptor context;
    
    /// The name of the type.
    TargetRelativeDirectPointer name;
    /// A pointer to the metadata access function for this type.
    int32_t /* ? */ accessFunctionPtr;
    /// A pointer to the field descriptor for the type, if any.
    TargetRelativeDirectPointer fields;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L3947
struct TargetClassDescriptor {
    struct TargetTypeContextDescriptor typeContext;
    
    /// The type of the superclass, expressed as a mangled type name that can
    /// refer to the generic arguments of the subclass type.
    TargetRelativeDirectPointer superclassType;
    
    uint32_t metadata;
    uint32_t extra;
    
    /// The number of additional members added by this class to the class
    /// metadata.  This data is opaque by default to the runtime, other than
    /// as exposed in other members; it's really just
    /// NumImmediateMembers * sizeof(void*) bytes of data.
    ///
    /// Whether those bytes are added before or after the address point
    /// depends on areImmediateMembersNegative().
    uint32_t numImmediateMembers;
    
    /// The number of stored properties in the class, not including its
    /// superclasses. If there is a field offset vector, this is its length.
    uint32_t numFields;
    
    /// The offset of the field offset vector for this class's stored
    /// properties in its metadata, in words. 0 means there is no field offset
    /// vector.
    ///
    /// If this class has a resilient superclass, this offset is relative to
    /// the size of the resilient superclass metadata. Otherwise, it is
    /// absolute.
    uint32_t fieldOffsetVectorOffset;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L4338
struct TargetValueTypeDescriptor {
    struct TargetTypeContextDescriptor typeContext;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L4349
struct TargetStructDescriptor {
    struct TargetValueTypeDescriptor valueType;
    
    /// The number of stored properties in the struct.
    /// If there is a field offset vector, this is its length.
    uint32_t numFields;
    /// The offset of the field offset vector for this struct's stored
    /// properties in its metadata, if any. 0 means there is no field offset
    /// vector.
    uint32_t fieldOffsetVectorOffset;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/ABI/Metadata.h#L4477
struct TargetEnumDescriptor {
    struct TargetValueTypeDescriptor valueType;
    
    struct __attribute__((__packed__)) {
        /// The number of non-empty cases in the enum
        uint32_t numPayloadCases : 24;
        /// The offset of the payload size in the metadata record in words
        uint32_t payloadSizeOffset : 8;
    } numPayloadCasesAndPayloadSizeOffset;
    
    /// The number of empty cases in the enum.
    uint32_t numEmptyCases;
};
_Static_assert(sizeof(((struct TargetEnumDescriptor *)0)->numPayloadCasesAndPayloadSizeOffset) == sizeof(uint32_t), "numPayloadCasesAndPayloadSizeOffset is backed by a uint32_t");

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/RemoteInspection/Records.h#L144
typedef enum : uint16_t {
    // Swift nominal types.
    TargetFieldDescriptorKindStruct,
    TargetFieldDescriptorKindClass,
    TargetFieldDescriptorKindEnum,
    
    // Fixed-size multi-payload enums have a special descriptor format that
    // encodes spare bits.
    //
    // For now, a descriptor with this kind
    // just means we also have a builtin descriptor from which we get the
    // size and alignment.
    TargetFieldDescriptorKindMultiPayloadEnum,
    
    /// A Swift opaque protocol. There are no fields, just a record for the
    /// type itself.
    TargetFieldDescriptorKindProtocol,
    
    /// A Swift class-bound protocol.
    TargetFieldDescriptorKindClassProtocol,
    
    /// An Objective-C protocol, which may be imported or defined in Swift.
    TargetFieldDescriptorKindObjCProtocol,
    
    /// An Objective-C class, which may be imported or defined in Swift.
    /// In the former case, field type metadata is not emitted, and
    /// must be obtained from the Objective-C runtime.
    TargetFieldDescriptorKindObjCClass
} TargetFieldDescriptorKind;

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/RemoteInspection/Records.h#L176
struct TargetFieldDescriptor {
    TargetRelativeDirectPointer mangledTypeName;
    TargetRelativeDirectPointer superclass;
    
    TargetFieldDescriptorKind kind;
    
    uint16_t fieldRecordSize;
    uint32_t numFields;
};

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/RemoteInspection/Records.h#L34
typedef enum : uint32_t {
    // Is this an indirect enum case?
    TargetFieldRecordFlagsIsIndirectCase = 0x1,
    // Is this a mutable `var` property?
    TargetFieldRecordFlagsIsVar = 0x2,
    // Is this an artificial field?
    TargetFieldRecordFlagsIsArtificial = 0x4,
} TargetFieldRecordFlags;

// https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/RemoteInspection/Records.h#L85
struct TargetFieldRecord {
    TargetFieldRecordFlags flags;
    
    TargetRelativeDirectPointer mangledTypeName;
    TargetRelativeDirectPointer fieldName;
};
