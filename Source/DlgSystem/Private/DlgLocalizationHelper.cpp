// // Copyright 2017-2019 Csaba Molnar, Daniel Butum
#include "DlgLocalizationHelper.h"

#include "DlgSystemSettings.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextPackageNamespaceUtil.h"
#include "Serialization/TextReferenceCollector.h"

void FDlgLocalizationHelper::UpdateTextNamespace(const UObject* Object, const UDlgSystemSettings* Settings, FText& Text)
{
#if !WITH_EDITOR
	return;
#endif

	const FString DefaultValue = TEXT("");

	// Culture invariant, empty, from string table, can't update idk
	if (!Text.ShouldGatherForLocalization())
	{
		return;
	}

	// Ignore from settings
	if (!Settings)
	{
		return;
	}
	if (Settings->DialogueTextLocalizationMode == EDlgTextLocalization::Ignore)
	{
		return;
	}

	const FString CurrentFullNamespace = FTextInspector::GetNamespace(Text).Get(DefaultValue);
	const FString CurrentNamespace = TextNamespaceUtil::StripPackageNamespace(CurrentFullNamespace);
	const FString CurrentKey = FTextInspector::GetKey(Text).Get(DefaultValue);

	// Get newer namespace, default is GlobalNamespace
	FString NewNamespace = Settings->DialogueTextNamespaceName;
	if (Settings->DialogueTextLocalizationMode == EDlgTextLocalization::NamespacePerDialogue)
	{
		if (!IsValid(Object))
		{
			return;
		}
		NewNamespace = Object->GetName();
	}

	// Nothing to change
	// Only apply the change if the new namespace/package is different - we want to keep the keys stable where possible
#if WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS
	{
		// Compare namespace + package
		const UPackage* Package = Object ? Object->GetOutermost() : nullptr;
		const FString PackageNamespace = TextNamespaceUtil::EnsurePackageNamespace(const_cast<UPackage*>(Package));
		const FString NewFullNamespace = TextNamespaceUtil::BuildFullNamespace(NewNamespace, PackageNamespace, /*bAlwaysApplyPackageNamespace*/true);
		if (CurrentFullNamespace.Equals(NewFullNamespace, ESearchCase::CaseSensitive))
		{
			return;
		}
	}
#else
	// Compare only namespaces without the package
	if (CurrentNamespace.Equals(NewNamespace, ESearchCase::CaseSensitive))
	{
		return;
	}
#endif
	 
	// We must use the package
#if WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS
	const FString* TextSource = FTextInspector::GetSourceString(Text);
	FString NewStableNamespace;
	FString NewStableKey;
	StaticStableTextId(
		Object,
		IEditableTextProperty::ETextPropertyEditAction::EditedNamespace,
		TextSource ? *TextSource : FString(),
		NewNamespace,
		CurrentKey,
		NewStableNamespace,
		NewStableKey
	);
	NewNamespace = NewStableNamespace;
#endif

	// Change namespace
	// Don't use this as this marks the text as immutable
	// Text = FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(*Text.ToString(), *NewNamespace, *CurrentKey);
#if WITH_EDITOR
	Text = FText::ChangeKey(NewNamespace, CurrentKey, Text);
#endif
}


#if WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS

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

#endif // WITH_EDITOR && USE_STABLE_LOCALIZATION_KEYS
