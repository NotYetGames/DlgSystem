// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgSystemSettings.h"

#include "GameFramework/Character.h"
#include "Runtime/Launch/Resources/Version.h"

#include "DlgManager.h"
#include "Logging/DlgLogger.h"

#define LOCTEXT_NAMESPACE "DlgSystem"

//////////////////////////////////////////////////////////////////////////
// UDlgSystemSettings
UDlgSystemSettings::UDlgSystemSettings()
{
	BlacklistedReflectionClasses = {AActor::StaticClass(), APawn::StaticClass(),  ACharacter::StaticClass()};
	// AdditionalTextFormatFileExtensionsToLookFor = {""};

	DefaultTextEdgeToEndNode = LOCTEXT("edge_finish", "Finish");
	DefaultTextEdgeToNormalNode = LOCTEXT("edge_next", "Next");
}

#if WITH_EDITOR
FText UDlgSystemSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "Dialogue");
}

FText UDlgSystemSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDescription", "Configure how the Dialogue Editor behaves + Runtime behaviour");
}


bool UDlgSystemSettings::CanEditChange(const FProperty* InProperty) const
{
	const bool bIsEditable = Super::CanEditChange(InProperty);
	if (bIsEditable && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();

		// Do now allow to change the bDrawPrimaryEdges, bDrawSecondaryEdges if we aren't even showing them
		if (!bShowPrimarySecondaryEdges &&
			(PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bDrawPrimaryEdges) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bDrawSecondaryEdges)))
		{
			return false;
		}

		// Only useful for GlobalNamespace
		if (DialogueTextNamespaceLocalization != EDlgTextNamespaceLocalization::Global &&
			PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, DialogueTextGlobalNamespaceName))
		{
			return false;
		}

		// Only useful for WithPrefixPerDialogue
		if (DialogueTextNamespaceLocalization != EDlgTextNamespaceLocalization::WithPrefixPerDialogue &&
			PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, DialogueTextPrefixNamespaceName))
		{
			return false;
		}

		if (DialogueTextNamespaceLocalization == EDlgTextNamespaceLocalization::Ignore &&
			(PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, LocalizationIgnoredStrings)))
		{
			return false;
		}

		// Can't edit because they are not used
		if (!bSetDefaultEdgeTexts &&
			(PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, DefaultTextEdgeToEndNode) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, DefaultTextEdgeToNormalNode) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bSetDefaultEdgeTextOnFirstChildOnly)))
		{
			return false;
		}
	}

	return bIsEditable;
}

void UDlgSystemSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bEnableMessageLog))
	{
		// Prevent no logging at all
		bEnableOutputLog = !bEnableMessageLog;
	}

	// Check category
	if (PropertyChangedEvent.Property != nullptr && PropertyChangedEvent.Property->HasMetaData(TEXT("Category")))
	{
		const FString& Category = PropertyChangedEvent.Property->GetMetaData(TEXT("Category"));

		// Sync logger settings
		if (Category.Equals(TEXT("Logger"), ESearchCase::IgnoreCase))
		{
			FDlgLogger::Get().SyncWithSettings();
		}
	}
}
#endif // WITH_EDITOR

bool UDlgSystemSettings::IsIgnoredTextForLocalization(const FText& Text) const
{
	// Ignored texts
	// for (const FText& Ignored : LocalizationIgnoredTexts)
	// {
	// 	if (Text.EqualTo(Ignored))
	// 	{
	// 		return false;
	// 	}
	// }

	// Ignore strings
	const FString& TextSourceString = *FTextInspector::GetSourceString(Text);
	if (LocalizationIgnoredStrings.Contains(TextSourceString))
	{
		return false;
	}
	// for (const FString& Ignored : LocalizationIgnoredStrings)
	// {
	// 	if (TextSourceString.Equals(Ignored))
	// 	{
	// 		return false;
	// 	}
	// }

	return true;
}

FString UDlgSystemSettings::GetTextFileExtension(EDlgDialogueTextFormat TextFormat)
{
	switch (TextFormat)
	{
		// JSON has the .json added at the end
		case EDlgDialogueTextFormat::JSON:
			return TEXT(".dlg.json");

		case EDlgDialogueTextFormat::DialogueDEPRECATED:
			return TEXT(".dlg");

		// Empty
		case EDlgDialogueTextFormat::None:
		default:
			return FString();
	}
}

const TSet<FString>& UDlgSystemSettings::GetAllCurrentTextFileExtensions()
{
	static TSet<FString> Extensions;
	if (Extensions.Num() == 0)
	{
		// Iterate over all possible text formats
		const int32 TextFormatsNum = static_cast<int32>(EDlgDialogueTextFormat::NumTextFormats);
		for (int32 TextFormatIndex = static_cast<int32>(EDlgDialogueTextFormat::StartTextFormats);
				   TextFormatIndex < TextFormatsNum; TextFormatIndex++)
		{
			const EDlgDialogueTextFormat CurrentTextFormat = static_cast<EDlgDialogueTextFormat>(TextFormatIndex);
			Extensions.Add(GetTextFileExtension(CurrentTextFormat));
		}
	}

	return Extensions;
}

TSet<FString> UDlgSystemSettings::GetAllTextFileExtensions() const
{
	TSet<FString> CurrentFileExtensions = GetAllCurrentTextFileExtensions();

	// Look for additional file extensions
	for (const FString& Ext : AdditionalTextFormatFileExtensionsToLookFor)
	{
		// Only allow file extension that start with dot, also ignore uasset
		if (Ext.StartsWith(".") && Ext != ".uasset")
		{
			CurrentFileExtensions.Add(Ext);
		}
	}

	return CurrentFileExtensions;
}

#undef LOCTEXT_NAMESPACE
