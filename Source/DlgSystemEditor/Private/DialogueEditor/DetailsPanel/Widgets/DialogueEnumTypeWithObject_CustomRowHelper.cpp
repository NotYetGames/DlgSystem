// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueEnumTypeWithObject_CustomRowHelper.h"

#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "DialogueEditableTextPropertyHandle.h"
#include "DlgHelper.h"
#include "Editor.h"
#include "SourceCodeNavigation.h"
#include "DialogueEditor/DetailsPanel/DialogueDetailsPanelUtils.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Images/SImage.h"


#define LOCTEXT_NAMESPACE "DialogueEnumTypeWithObject_CustomRowHelper"
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

bool FDialogueEnumTypeWithObject_CustomRowHelper::CanBeVisible() const
{
	if (!HasParticipantName() || !Dialogue.IsValid() || !GetObject())
	{
		return false;
	}

	if (EnumType == EDialogueEnumWithObjectType::Condition)
	{
		return CanBeVisibleForConditionType();
	}
	if (EnumType == EDialogueEnumWithObjectType::Event)
	{
		return CanBeVisibleForEventType();
	}

	return false;
}

UObject* FDialogueEnumTypeWithObject_CustomRowHelper::GetObject() const
{
	if (!Dialogue.IsValid())
	{
		return nullptr;
	}
	const FName ParticipantName = GetParticipantName();
	if (ParticipantName == NAME_None)
	{
		return nullptr;
	}

	UClass* Class = Dialogue->GetParticipantClass(ParticipantName);
	if (!Class)
	{
		return nullptr;
	}

	return Class->GetDefaultObject();
}

FText FDialogueEnumTypeWithObject_CustomRowHelper::GetBrowseObjectText() const
{
	return LOCTEXT("BrowseButtonToolTipText", "Browse Participant Asset in Content Browser");
}

FText FDialogueEnumTypeWithObject_CustomRowHelper::GetJumpToObjectText() const
{
	if (IsObjectABlueprint())
	{
		return LOCTEXT("OpenObjectBlueprintTooltipKey", "Open Participant Blueprint Editor");
	}

	// Native Class
	return FText::Format(
		LOCTEXT("OpenObjectBlueprintTooltipKey", "Open Participant Source File in {0}"),
		FSourceCodeNavigation::GetSelectedSourceCodeIDE()
	);
}

FReply FDialogueEnumTypeWithObject_CustomRowHelper::OnOpenClicked()
{
	// Set function/event name to open
	if (EnumType == EDialogueEnumWithObjectType::Condition)
	{
		SetFunctionNameToOpen(EDialogueBlueprintOpenType::Function, FDlgHelper::GetFunctionNameForConditionType(GetConditionType()));
	}
	if (EnumType == EDialogueEnumWithObjectType::Event)
	{
		SetFunctionNameToOpen(EDialogueBlueprintOpenType::Function, FDlgHelper::GetFunctionNameForEventType(GetEventType()));
	}

	return Super::OnOpenClicked();
}

uint8 FDialogueEnumTypeWithObject_CustomRowHelper::GetEnumValue() const
{
	if (!PropertyRow)
	{
		return 0;
	}

	uint8 Value = 0;
	TSharedPtr<IPropertyHandle> PropertyHandle = PropertyRow->GetPropertyHandle();
	if (!PropertyHandle.IsValid())
	{
		return 0;
	}
	PropertyHandle->GetValue(Value);
	return Value;
}

FName FDialogueEnumTypeWithObject_CustomRowHelper::GetParticipantName() const
{
	if (!ParticipantNamePropertyHandle.IsValid())
	{
		return NAME_None;
	}

	return FDialogueDetailsPanelUtils::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
#undef DEFAULT_FONT
