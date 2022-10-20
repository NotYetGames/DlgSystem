// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "Launch/Resources/Version.h"

#define NY_ENGINE_VERSION (ENGINE_MAJOR_VERSION * 100 + ENGINE_MINOR_VERSION)

#if NY_ENGINE_VERSION < 425
	// Mimic 4.25 Reflection system changes
	using FFieldClass = UClass;
	using FProperty = UProperty;

	using FNumericProperty = UNumericProperty;
	using FByteProperty = UByteProperty;
	using FIntProperty = UIntProperty;
	using FInt64Property = UInt64Property;

	using FFloatProperty = UFloatProperty;
	using FDoubleProperty = UDoubleProperty;
	using FBoolProperty = UBoolProperty;

	using FObjectPropertyBase = UObjectPropertyBase;
	using FObjectProperty = UObjectProperty;
	using FWeakObjectProperty = UWeakObjectProperty;
	using FLazyObjectProperty = ULazyObjectProperty;
	using FSoftObjectProperty = USoftObjectProperty;
	using FClassProperty = UClassProperty;
	using FSoftClassProperty = USoftClassProperty;
	using FInterfaceProperty = UInterfaceProperty;

	using FNameProperty = UNameProperty;
	using FStrProperty = UStrProperty;
	using FTextProperty = UTextProperty;

	using FArrayProperty = UArrayProperty;
	using FMapProperty = UMapProperty;
	using FSetProperty = USetProperty;

	using FStructProperty = UStructProperty;
	// using FMulticastDelegateProperty FNYMulticastDelegateProperty;
	// using FMulticastInlineDelegateProperty FNYMulticastInlineDelegateProperty;
	// using FMulticastSparseDelegateProperty FNYMulticastSparseDelegateProperty;
	using FEnumProperty = UEnumProperty;
#endif // NY_ENGINE_VERSION < 425

#if NY_ENGINE_VERSION >= 424
	#define NY_ARRAY_COUNT UE_ARRAY_COUNT
#else
	#define NY_ARRAY_COUNT ARRAY_COUNT
#endif

#if WITH_EDITOR
	#if NY_ENGINE_VERSION >= 501
		using FNYAppStyle = FAppStyle;
		#define NY_GET_APP_STYLE_NAME() FNYAppStyle::GetAppStyleSetName()
	#else
		#include "EditorStyleSet.h"
		using FNYAppStyle = FEditorStyle;
		#define NY_GET_APP_STYLE_NAME() FNYAppStyle::GetStyleSetName()
	#endif // NY_ENGINE_VERSION >= 501
#endif // WITH_EDITOR
