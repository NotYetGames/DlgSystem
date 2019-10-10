// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueEvent_Details.h"

#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"

#include "Nodes/DlgNode.h"
#include "DlgReflectionHelper.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "Widgets/SDialogueTextPropertyPickList.h"
#include "Widgets/DialogueTextPropertyPickList_CustomRowHelper.h"
#include "DlgHelper.h"

#define LOCTEXT_NAMESPACE "DialogueEvent_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueEventCustomization
void FDialogueEvent_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDialogueDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	PropertyUtils = StructCustomizationUtils.GetPropertyUtilities();

	// Cache the Property Handle for the EventType
	ParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, ParticipantName));
	EventTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, EventType));
	check(ParticipantNamePropertyHandle.IsValid());
	check(EventTypePropertyHandle.IsValid());

	// Register handler for event type change
	EventTypePropertyHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &Self::OnEventTypeChanged, true));

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDialogueEvent_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const bool bHasDialogue = Dialogue != nullptr;


	// Common ParticipantName
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		.Update();
	}

	// EventType
	StructBuilder.AddProperty(EventTypePropertyHandle.ToSharedRef());

	// EventName
	{
		const TSharedPtr<IPropertyHandle> EventNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, EventName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("EventNameSearchKey", "Event Name"));

		EventNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, EventNamePropertyHandle);
		EventNamePropertyRow->SetTextPropertyPickListWidget(
				SNew(SDialogueTextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetAllDialoguesEventNames)
				.OnTextCommitted(this, &Self::HandleTextCommitted)
				.HasContextCheckbox(bHasDialogue)
				.IsContextCheckBoxChecked(false)
				.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueEventNames)
		);
		EventNamePropertyRow->Update();
	}

	// IntValue
	{
		IntValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, IntValue)).ToSharedRef());
		IntValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetIntValueVisibility));
	}

	// FloatValue
	{
		FloatValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, FloatValue)).ToSharedRef());
		FloatValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetFloatValueVisibility));
	}

	// NameValue
	{
		NameValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, NameValue)).ToSharedRef());
		NameValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetNameValueVisibility));
	}

	// bDelta
	{
		BoolDeltaPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, bDelta)).ToSharedRef());
		BoolDeltaPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolDeltaVisibility));
	}

	// bValue
	{
		BoolValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, bValue)).ToSharedRef());
		BoolValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolValueVisibility));
	}

	// Cache the initial event type
	OnEventTypeChanged(false);
}

void FDialogueEvent_Details::OnEventTypeChanged(bool bForceRefresh)
{
	// Update to the new type
	uint8 Value = 0;
	if (EventTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}
	EventType = static_cast<EDlgEventType>(Value);

	// Update the display name/tooltips
	FText EventNameDisplayName = LOCTEXT("EventNameDisplayName", "Variable Name");
	FText EventNameToolTip = LOCTEXT("EventNameToolTip", "Name of the relevant variable");
	if (EventType == EDlgEventType::Event)
	{
		EventNameDisplayName = LOCTEXT("DlgEvent_EventNameDisplayName", "Event Name");
		EventNameToolTip = LOCTEXT("DlgEvent_EventNameToolTip", "Name of the relevant event");
	}

	EventNamePropertyRow->SetDisplayName(EventNameDisplayName)
		.SetToolTip(EventNameToolTip)
		.Update();

	// Refresh the view, without this some names/tooltips won't get refreshed
	if (bForceRefresh && PropertyUtils.IsValid())
	{
		PropertyUtils->ForceRefresh();
	}
}

TArray<FName> FDialogueEvent_Details::GetAllDialoguesEventNames() const
{
	TArray<FName> Suggestions;
	const FName ParticipantName = FDialogueDetailsPanelUtils::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());

	switch (EventType)
	{
	case EDlgEventType::ModifyBool:
		UDlgManager::GetAllDialoguesBoolNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyFloat:
		UDlgManager::GetAllDialoguesFloatNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyInt:
		UDlgManager::GetAllDialoguesIntNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyName:
		UDlgManager::GetAllDialoguesNameNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyClassIntVariable:
		if (Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UIntProperty::StaticClass(), Suggestions);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;

	case EDlgEventType::ModifyClassFloatVariable:
		if (Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UFloatProperty::StaticClass(), Suggestions);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;

	case EDlgEventType::ModifyClassBoolVariable:
		if (Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UBoolProperty::StaticClass(), Suggestions);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;

	case EDlgEventType::ModifyClassNameVariable:
		if (Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UNameProperty::StaticClass(), Suggestions);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;


	case EDlgEventType::Event:
	default:
		UDlgManager::GetAllDialoguesEventNames(ParticipantName, Suggestions);
		break;
	}

	return Suggestions;
}

TArray<FName> FDialogueEvent_Details::GetCurrentDialogueEventNames() const
{
	if (Dialogue == nullptr)
	{
		return {};
	}

	const FName ParticipantName = FDialogueDetailsPanelUtils::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());
	TSet<FName> Suggestions;

	switch (EventType)
	{
	case EDlgEventType::ModifyBool:
		Dialogue->GetBoolNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyName:
		Dialogue->GetNameNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyFloat:
		Dialogue->GetFloatNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyInt:
		Dialogue->GetIntNames(ParticipantName, Suggestions);
		break;

	case EDlgEventType::ModifyClassIntVariable:
		UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UIntProperty::StaticClass(), Suggestions);
		break;

	case EDlgEventType::ModifyClassFloatVariable:
		UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UFloatProperty::StaticClass(), Suggestions);
		break;

	case EDlgEventType::ModifyClassBoolVariable:
		UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UBoolProperty::StaticClass(), Suggestions);
		break;

	case EDlgEventType::ModifyClassNameVariable:
		UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UNameProperty::StaticClass(), Suggestions);
		break;

	case EDlgEventType::Event:
	default:
		Dialogue->GetEvents(ParticipantName, Suggestions);
		break;
	}

	FDlgHelper::SortDefault(Suggestions);
	return Suggestions.Array();
}

#undef LOCTEXT_NAMESPACE
