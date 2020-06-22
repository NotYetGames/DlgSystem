// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
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
	static void UpdateTextFromRemapping(const UDlgSystemSettings& Settings, FText& OutText);

#if WITH_EDITOR
	// Updates the text namespace to match the settings options
	// NOTE: only works in editor mode
	static void UpdateTextNamespaceAndKey(const UObject* Object, const UDlgSystemSettings& Settings, FText& Text);
#else
	// NO OP
	static void UpdateTextNamespaceAndKey(const UObject* Object, const UDlgSystemSettings& Settings, FText& Text) {}
#endif

	// Will we update the text namespace for the following texts
	static bool WillTextNamespaceBeUpdated(const FText& Text);
	static bool WillTextNamespaceBeUpdated(const FText& Text, const UDlgSystemSettings& Settings);

#if WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS
	// Copied From IEditableTextProperty

	/** Get the localization ID we should use for the given object, and the given text instance */
	static void StaticStableTextId(const UObject* InObject, IEditableTextProperty::ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey);

	/** Get the localization ID we should use for the given package, and the given text instance */
	static void StaticStableTextId(const UPackage* InPackage, IEditableTextProperty::ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey);

protected:
	// Returns true if we should do an update of the FText
	//         false otherwise
	static bool GetNewNamespaceAndKey(
		const UObject* Object,
		const UDlgSystemSettings& Settings,
		const FText& Text,
		FString& OutNewNamespace,
		FString& OutNewKey
	);

#endif // WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS

};
