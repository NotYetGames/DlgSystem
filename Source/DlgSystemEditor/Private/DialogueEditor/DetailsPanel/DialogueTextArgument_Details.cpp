// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueTextArgument_Details.h"

#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"

#include "DlgReflectionHelper.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "STextPropertyPickList.h"
#include "CustomRowHelpers/TextPropertyPickList_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueTextArgument_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueEventCustomization
void FDialogueTextArgument_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDialogueDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	PropertyUtils = StructCustomizationUtils.GetPropertyUtilities();

	// Cache the Property Handle for the ArgumentType
	ParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgTextArgument, ParticipantName));
	ArgumentTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgTextArgument, Type));
	check(ParticipantNamePropertyHandle.IsValid());
	check(ArgumentTypePropertyHandle.IsValid());

	// Register handler for event type change
	ArgumentTypePropertyHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &Self::OnArgumentTypeChanged, true));

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDialogueTextArgument_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const bool bHasDialogue = Dialogue != nullptr;

	// DisplayString
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgTextArgument, DisplayString)).ToSharedRef());

	// ParticipantName
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		->Update();
	}

	// ArgumentType
	StructBuilder.AddProperty(ArgumentTypePropertyHandle.ToSharedRef());

	// VariableName
	{
		const TSharedPtr<IPropertyHandle> VariableNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgTextArgument, VariableName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("VariableNameSearchKey", "Variable Name"));

		VariableNamePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, VariableNamePropertyHandle);
		VariableNamePropertyRow->SetTextPropertyPickListWidget(
				SNew(STextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetAllDialoguesVariableNames)
				.OnTextCommitted(this, &Self::HandleTextCommitted)
				.HasContextCheckbox(bHasDialogue)
				.IsContextCheckBoxChecked(false)
				.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueVariableNames)
		);
		VariableNamePropertyRow->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetVariableNameVisibility));
		VariableNamePropertyRow->Update();
	}

	// Cache the initial event type
	OnArgumentTypeChanged(false);
}

void FDialogueTextArgument_Details::OnArgumentTypeChanged(bool bForceRefresh)
{
	// Update to the new type
	uint8 Value = 0;
	if (ArgumentTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}
	ArgumentType = static_cast<EDlgTextArgumentType>(Value);

	// Refresh the view, without this some names/tooltips won't get refreshed
	if (bForceRefresh && PropertyUtils.IsValid())
	{
		PropertyUtils->ForceRefresh();
	}
}

TArray<FName> FDialogueTextArgument_Details::GetDialogueVariableNames(bool bCurrentOnly) const
{
	TArray<FName> Suggestions;
	const FName ParticipantName = FDialogueDetailsPanelUtils::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());

	switch (ArgumentType)
	{
		case EDlgTextArgumentType::DlgTextArgumentDialogueInt:
			if (bCurrentOnly && Dialogue)
			{
				TSet<FName> SuggestionsSet;
				Dialogue->GetIntNames(ParticipantName, SuggestionsSet);
				Suggestions = SuggestionsSet.Array();
			}
			else
			{
				UDlgManager::GetAllDialoguesIntNames(ParticipantName, Suggestions);
			}
			break;

		case EDlgTextArgumentType::DlgTextArgumentClassInt:
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UIntProperty::StaticClass(), Suggestions);
			break;

		case EDlgTextArgumentType::DlgTextArgumentDialogueFloat:
			if (bCurrentOnly && Dialogue)
			{
				TSet<FName> SuggestionsSet;
				Dialogue->GetFloatNames(ParticipantName, SuggestionsSet);
				Suggestions = SuggestionsSet.Array();
			}
			else
			{
				UDlgManager::GetAllDialoguesFloatNames(ParticipantName, Suggestions);
			}
			break;

		case EDlgTextArgumentType::DlgTextArgumentClassFloat:
			if (Dialogue)
			{
				UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UFloatProperty::StaticClass(), Suggestions);
			}
			break;

		case EDlgTextArgumentType::DlgTextArgumentClassText:
			if (Dialogue)
			{
				UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UTextProperty::StaticClass(), Suggestions);
			}
			break;

		case EDlgTextArgumentType::DlgTextArgumentDisplayName:
		default:
			break;
	}
	FDlgHelper::SortDefault(Suggestions);
	return Suggestions;
}

#undef LOCTEXT_NAMESPACE
