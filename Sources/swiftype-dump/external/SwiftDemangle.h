// copied from https://github.com/swiftlang/swift/blob/48015abbd89116163115e1d728e8478829c91271/include/swift/SwiftDemangle/SwiftDemangle.h

//===--- SwiftDemangle.h - Public demangling interface ----------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header declares functions in the libswiftDemangle library,
/// which provides external access to Swift's demangler.
///
//===----------------------------------------------------------------------===//

#ifndef SWIFT_DEMANGLE_SWIFT_DEMANGLE_H
#define SWIFT_DEMANGLE_SWIFT_DEMANGLE_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(swiftDemangle_EXPORTS)
# if defined(__ELF__)
#   define SWIFT_DEMANGLE_LINKAGE __attribute__((__visibility__("protected")))
# elif defined(__MACH__)
#   define SWIFT_DEMANGLE_LINKAGE __attribute__((__visibility__("default")))
# else
#   define SWIFT_DEMANGLE_LINKAGE __declspec(dllexport)
# endif
#else
# if defined(__ELF__)
#   define SWIFT_DEMANGLE_LINKAGE __attribute__((__visibility__("default")))
# elif defined(__MACH__)
#   define SWIFT_DEMANGLE_LINKAGE __attribute__((__visibility__("default")))
# else
#   define SWIFT_DEMANGLE_LINKAGE __declspec(dllimport)
# endif
#endif

#if defined(__cplusplus)
}
#endif


/// @{
/// Version constants for libswiftDemangle library.

/// Major version changes when there are ABI or source incompatible changes.
#define SWIFT_DEMANGLE_VERSION_MAJOR 1

/// Minor version changes when new APIs are added in ABI- and source-compatible
/// way.
#define SWIFT_DEMANGLE_VERSION_MINOR 2

/// @}

#ifdef __cplusplus
extern "C" {
#endif

/// Demangle Swift function names.
///
/// \returns the length of the demangled function name (even if greater than the
/// size of the output buffer) or 0 if the input is not a Swift-mangled function
/// name (in which cases \p OutputBuffer is left untouched).
SWIFT_DEMANGLE_LINKAGE
size_t swift_demangle_getDemangledName(const char *MangledName,
                                       char *OutputBuffer, size_t Length);

/// Demangle Swift function names with module names and implicit self
/// and metatype type names in function signatures stripped.
///
/// \returns the length of the demangled function name (even if greater than the
/// size of the output buffer) or 0 if the input is not a Swift-mangled function
/// name (in which cases \p OutputBuffer is left untouched).
SWIFT_DEMANGLE_LINKAGE
size_t swift_demangle_getSimplifiedDemangledName(const char *MangledName,
                                                 char *OutputBuffer,
                                                 size_t Length);

/// Demangle a Swift symbol and return the module name of the mangled entity.
///
/// \returns the length of the demangled module name (even if greater than the
/// size of the output buffer) or 0 if the input is not a Swift-mangled name
/// (in which cases \p OutputBuffer is left untouched).
SWIFT_DEMANGLE_LINKAGE
size_t swift_demangle_getModuleName(const char *MangledName,
                                    char *OutputBuffer,
                                    size_t Length);

/// Demangles a Swift function name and returns true if the function
/// conforms to the Swift calling convention.
///
/// \returns true if the function conforms to the Swift calling convention.
/// The return value is unspecified if the \p MangledName does not refer to a
/// function symbol.
SWIFT_DEMANGLE_LINKAGE
int swift_demangle_hasSwiftCallingConvention(const char *MangledName);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SWIFT_DEMANGLE_SWIFT_DEMANGLE_H

