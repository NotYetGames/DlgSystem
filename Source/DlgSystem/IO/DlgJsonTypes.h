// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

#include "DlgSystem/NYEngineVersionHelpers.h"

#if NY_ENGINE_VERSION >= 508
using FNYJsonObjectKey = FJsonObject::FStringType;
#else
using FNYJsonObjectKey = FString;
#endif

using FNYJsonAttributes = TMap<FNYJsonObjectKey, TSharedPtr<FJsonValue>>;

inline FNYJsonObjectKey FNYMakeJsonObjectKey(const TCHAR* Key)
{
#if NY_ENGINE_VERSION >= 508
	return FNYJsonObjectKey(FStringView(Key));
#else
	return FString(Key);
#endif
}

inline FNYJsonObjectKey FNYMakeJsonObjectKey(const FString& Key)
{
#if NY_ENGINE_VERSION >= 508
	return FNYJsonObjectKey(FStringView(*Key, Key.Len()));
#else
	return Key;
#endif
}

inline FString FNYJsonObjectKeyToString(const FNYJsonObjectKey& Key)
{
#if NY_ENGINE_VERSION >= 508
	return FString(Key.Len(), *Key);
#else
	return Key;
#endif
}
