// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Launch/Resources/Version.h"

// Protect against inclusion in multiple projects
#ifndef NY_REFLECTION_TYPES
#define NY_REFLECTION_TYPES

#if ENGINE_MINOR_VERSION >= 25
// 4.25 Reflection system changes

typedef FFieldClass FNYPropertyClass;
typedef FProperty FNYProperty;

typedef FNumericProperty FNYNumericProperty;
typedef FByteProperty FNYByteProperty;
typedef FInt8Property FNYInt8Property;
typedef FInt16Property FNYInt16Property;
typedef FInt16Property FNYInt16Property;
typedef FIntProperty FNYIntProperty;
typedef FInt64Property FNYInt64Property;
typedef FUInt16Property FNYUInt16Property;
typedef FUInt32Property FNYUInt32Property;
typedef FUInt64Property FNYUInt64Property;
typedef FFloatProperty FNYFloatProperty;
typedef FDoubleProperty FNYDoubleProperty;
typedef FBoolProperty FNYBoolProperty;

typedef FObjectPropertyBase FNYObjectPropertyBase;
typedef FObjectProperty FNYObjectProperty;
typedef FWeakObjectProperty FNYWeakObjectProperty;
typedef FLazyObjectProperty FNYLazyObjectProperty;
typedef FSoftObjectProperty FNYSofObjectProperty;
typedef FClassProperty FNYClassProperty;
typedef FSoftClassProperty FNYSoftClassProperty;
typedef FInterfaceProperty FNYInterfaceProperty;

typedef FNameProperty FNYNameProperty;
typedef FStrProperty FNYStrProperty;
typedef FTextProperty FNYTextProperty;

typedef FArrayProperty FNYArrayProperty;
typedef FMapProperty FNYMapProperty;
typedef FSetProperty FNYSetProperty;

typedef FStructProperty FNYStructProperty;
// typedef FMulticastDelegateProperty FNYMulticastDelegateProperty;
// typedef FMulticastInlineDelegateProperty FNYMulticastInlineDelegateProperty;
// typedef FMulticastSparseDelegateProperty FNYMulticastSparseDelegateProperty;
typedef FEnumProperty FNYEnumProperty;

#else
// 4.24 and before where properties where UObjects

typedef UClass FNYPropertyClass;
typedef UProperty FNYProperty;

typedef UNumericProperty FNYNumericProperty;
typedef UByteProperty FNYByteProperty;
typedef UInt8Property FNYInt8Property;
typedef UInt16Property FNYInt16Property;
typedef UInt16Property FNYInt16Property;
typedef UIntProperty FNYIntProperty;
typedef UInt64Property FNYInt64Property;
typedef UUInt16Property FNYUInt16Property;
typedef UUInt32Property FNYUInt32Property;
typedef UUInt64Property FNYUInt64Property;
typedef UFloatProperty FNYFloatProperty;
typedef UDoubleProperty FNYDoubleProperty;
typedef UBoolProperty FNYBoolProperty;

typedef UObjectPropertyBase FNYObjectPropertyBase;
typedef UObjectProperty FNYObjectProperty;
typedef UWeakObjectProperty FNYWeakObjectProperty;
typedef ULazyObjectProperty FNYLazyObjectProperty;
typedef USoftObjectProperty FNYSofObjectProperty;
typedef UClassProperty FNYClassProperty;
typedef USoftClassProperty FNYSoftClassProperty;
typedef UInterfaceProperty FNYInterfaceProperty;

typedef UNameProperty FNYNameProperty;
typedef UStrProperty FNYStrProperty;
typedef UTextProperty FNYTextProperty;

typedef UArrayProperty FNYArrayProperty;
typedef UMapProperty FNYMapProperty;
typedef USetProperty FNYSetProperty;

typedef UStructProperty FNYStructProperty;
// typedef UMulticastDelegateProperty FNYMulticastDelegateProperty;
// typedef UMulticastInlineDelegateProperty FNYMulticastInlineDelegateProperty;
// typedef UMulticastSparseDelegateProperty FNYMulticastSparseDelegateProperty;
typedef UEnumProperty FNYEnumProperty;

#endif //  ENGINE_MINOR_VERSION >= 25


#endif // NY_REFLECTION_TYPES
