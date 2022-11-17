// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgLocalizationHelper.h"

#include "DlgSystemSettings.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextPackageNamespaceUtil.h"
#include "Serialization/TextReferenceCollector.h"


bool FDlgLocalizationHelper::WillTextNamespaceBeUpdated(const FText& Text)
{
	const auto* Settings = GetDefault<UDlgSystemSettings>();
	return WillTextNamespaceBeUpdated(Text, *Settings);
}

bool FDlgLocalizationHelper::WillTextNamespaceBeUpdated(const FText& Text, const UDlgSystemSettings& Settings)
{
	// Means we can override it
	return Settings.DialogueTextNamespaceLocalization != EDlgTextNamespaceLocalization::Ignore &&
		   Settings.IsIgnoredTextForLocalization(Text);
}

void FDlgLocalizationHelper::UpdateTextFromRemapping(const UDlgSystemSettings& Settings, FText& OutText)
{
	if (Settings.IsTextRemapped(OutText))
	{
		// Remapped
		const FText& RemappedText = Settings.GetTextRemappedText(OutText);
		OutText = RemappedText;
		// Copy namespace and key
		// NewNamespace = FTextInspector::GetNamespace(RemappedText).Get(DefaultValue);
		// NewKey = FTextInspector::GetKey(RemappedText).Get(DefaultValue);
	}
}

#if WITH_EDITOR

void FDlgLocalizationHelper::UpdateTextNamespaceAndKey(const UObject* Object, const UDlgSystemSettings& Settings, FText& Text)
{
	static const FString DefaultValue = TEXT("");

	// See if we can edit this
	if (!IsValid(Object))
	{
		return;
	}

	// Text remapping takes precedence over everything
	FString NewNamespace;
	FString NewKey;
	if (!GetNewNamespaceAndKey(Object, Settings, Text, NewNamespace, NewKey))
	{
		return;
	}

	// Change namespace
	// Don't use this as this marks the text as immutable
	// Text = FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(*Text.ToString(), *NewNamespace, *CurrentKey);
	Text = FText::ChangeKey(NewNamespace, NewKey, Text);
}

bool FDlgLocalizationHelper::GetNewNamespaceAndKey(
	const UObject* Object,
	const UDlgSystemSettings& Settings,
	const FText& Text,
	FString& OutNewNamespace,
	FString& OutNewKey
)
{
	static const FString DefaultValue = TEXT("");
	if (Settings.DialogueTextNamespaceLocalization == EDlgTextNamespaceLocalization::Ignore)
	{
		return false;
	}
	// Culture invariant, empty, from string table, can't update idk
	if (!Text.ShouldGatherForLocalization())
	{
		return false;
	}
	if (!Settings.IsIgnoredTextForLocalization(Text))
	{
		return false;
	}

	bool bNamespaceChanged = false;
	bool bKeyChanged = false;

	const FString CurrentFullNamespace = FTextInspector::GetNamespace(Text).Get(DefaultValue);
	const FString CurrentKey = FTextInspector::GetKey(Text).Get(DefaultValue);
	FString NewKey = CurrentKey;

	// Set new Namespace
	FString NewNamespace = Settings.DialogueTextGlobalNamespaceName; // GlobalNamespace
	if (Settings.DialogueTextNamespaceLocalization == EDlgTextNamespaceLocalization::PerDialogue)
	{
		NewNamespace = Object->GetName();
	}
	else if (Settings.DialogueTextNamespaceLocalization == EDlgTextNamespaceLocalization::WithPrefixPerDialogue)
	{
		NewNamespace = Settings.DialogueTextPrefixNamespaceName + Object->GetName();
	}

	// Did namespace change?
	// Only apply the change if the new namespace/package is different - we want to keep the keys stable where possible
#if USE_STABLE_LOCALIZATION_KEYS
	{
		// Compare namespace + package
		const UPackage* Package = Object ? Object->GetOutermost() : nullptr;
		const FString PackageNamespace = TextNamespaceUtil::EnsurePackageNamespace(const_cast<UPackage*>(Package));
		const FString NewFullNamespace = TextNamespaceUtil::BuildFullNamespace(NewNamespace, PackageNamespace, /*bAlwaysApplyPackageNamespace*/true);
		bNamespaceChanged = !CurrentFullNamespace.Equals(NewFullNamespace, ESearchCase::CaseSensitive);
	}
#else
	// Compare only namespaces without the package
	const FString CurrentNamespace = TextNamespaceUtil::StripPackageNamespace(CurrentFullNamespace);
	bNamespaceChanged = !CurrentNamespace.Equals(NewNamespace, ESearchCase::CaseSensitive);
#endif // USE_STABLE_LOCALIZATION_KEYS

	// Did key change?
	//bKeyChanged = !CurrentKey.Equals(NewKey, ESearchCase::CaseSensitive);

	// Get stabilized namespace and keys
	// We must use the package to get a stabilized key
#if USE_STABLE_LOCALIZATION_KEYS
	if (bNamespaceChanged)
	{
		// NOTE: dereference should be legal because GetSourceString always return a valid pointer
		const FString& TextSource = *FTextInspector::GetSourceString(Text);
		FString NewStableNamespace;
		FString NewStableKey;
		StaticStableTextId(
			Object,
			IEditableTextProperty::ETextPropertyEditAction::EditedNamespace,
			TextSource,
			NewNamespace,
			NewKey,
			NewStableNamespace,
			NewStableKey
		);
		NewNamespace = NewStableNamespace;
		NewKey = NewStableKey;
	}
#endif // USE_STABLE_LOCALIZATION_KEYS

	// Did key change?
	bKeyChanged = !CurrentKey.Equals(NewKey, ESearchCase::CaseSensitive);

	// Something changed
	if (bNamespaceChanged || bKeyChanged)
	{
		OutNewNamespace = NewNamespace;
		OutNewKey = NewKey;
		return true;
	}

	return false;
}


#if USE_STABLE_LOCALIZATION_KEYS

void FDlgLocalizationHelper::StaticStableTextId(const UObject* InObject, IEditableTextProperty::ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey)
{
	const UPackage* Package = InObject ? InObject->GetOutermost() : nullptr;
	StaticStableTextId(Package, InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
}

void FDlgLocalizationHelper::StaticStableTextId(const UPackage* InPackage, IEditableTextProperty::ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey)
{
	bool bPersistKey = false;

	const FString PackageNamespace = TextNamespaceUtil::EnsurePackageNamespace(const_cast<UPackage*>(InPackage));
	if (!PackageNamespace.IsEmpty())
	{
		// Make sure the proposed namespace is using the correct namespace for this package
		OutStableNamespace = TextNamespaceUtil::BuildFullNamespace(InProposedNamespace, PackageNamespace, /*bAlwaysApplyPackageNamespace*/true);

		if (InProposedNamespace.Equals(OutStableNamespace, ESearchCase::CaseSensitive) || InEditAction == IEditableTextProperty::ETextPropertyEditAction::EditedNamespace)
		{
			// If the proposal was already using the correct namespace (or we just set the namespace), attempt to persist the proposed key too
			if (!InProposedKey.IsEmpty())
			{
				// If we changed the source text, then we can persist the key if this text is the *only* reference using that ID
				// If we changed the identifier, then we can persist the key only if doing so won't cause an identify conflict
				const FTextReferenceCollector::EComparisonMode ReferenceComparisonMode = InEditAction == IEditableTextProperty::ETextPropertyEditAction::EditedSource ? FTextReferenceCollector::EComparisonMode::MatchId : FTextReferenceCollector::EComparisonMode::MismatchSource;
				const int32 RequiredReferenceCount = InEditAction == IEditableTextProperty::ETextPropertyEditAction::EditedSource ? 1 : 0;

				int32 ReferenceCount = 0;
				FTextReferenceCollector(InPackage, ReferenceComparisonMode, OutStableNamespace, InProposedKey, InTextSource, ReferenceCount);

				if (ReferenceCount == RequiredReferenceCount)
				{
					bPersistKey = true;
					OutStableKey = InProposedKey;
				}
			}
		}
		else if (InEditAction != IEditableTextProperty::ETextPropertyEditAction::EditedNamespace)
		{
			// If our proposed namespace wasn't correct for our package, and we didn't just set it (which doesn't include the package namespace)
			// then we should clear out any user specified part of it
			OutStableNamespace = TextNamespaceUtil::BuildFullNamespace(FString(), PackageNamespace, /*bAlwaysApplyPackageNamespace*/true);
		}
	}

	if (!bPersistKey)
	{
		OutStableKey = FGuid::NewGuid().ToString();
	}
}

#endif // USE_STABLE_LOCALIZATION_KEYS

#endif // WITH_EDITOR
