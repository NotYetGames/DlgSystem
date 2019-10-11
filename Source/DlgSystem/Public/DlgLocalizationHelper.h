// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS
#include "STextPropertyEditableTextBox.h"
#endif// WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS

class UDlgSystemSettings;

/**
 * General helper methods
 */
class DLGSYSTEM_API FDlgLocalizationHelper
{
	typedef FDlgLocalizationHelper Self;
public:
	// Updates the text namespace to match the settings options
	// NOTE: only works in editor mode
	static void UpdateTextNamespace(const UObject* Object, const UDlgSystemSettings* Settings, FText& Text);
	
#if WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS
	// From IEditableTextProperty

	enum class EDlgTextPropertyEditAction : uint8
	{
		EditedNamespace,
		EditedKey,
		EditedSource,
	};
	
	/** Get the localization ID we should use for the given object, and the given text instance */
	static void StaticStableTextId(const UObject* InObject, IEditableTextProperty::ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey);

	/** Get the localization ID we should use for the given package, and the given text instance */
	static void StaticStableTextId(const UPackage* InPackage, IEditableTextProperty::ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey);
#endif // WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS
};
