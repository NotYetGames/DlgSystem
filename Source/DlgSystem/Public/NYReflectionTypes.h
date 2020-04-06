// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

// Protect against inclusion in multiple projects
#ifndef NY_REFLECTION_TYPES
#define NY_REFLECTION_TYPES

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
typedef UMulticastDelegateProperty FNYMulticastDelegateProperty;
typedef UMulticastInlineDelegateProperty FNYMulticastInlineDelegateProperty;
typedef UMulticastSparseDelegateProperty FNYMulticastSparseDelegateProperty;
typedef UEnumProperty FNYEnumProperty;


#endif // NY_REFLECTION_TYPES
