// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEvent_Details.h"

#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"

#include "DlgSystem/NYReflectionHelper.h"
#include "DlgDetailsPanelUtils.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/SDlgTextPropertyPickList.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgTextPropertyPickList_CustomRowHelper.h"
#include "DlgSystem/DlgHelper.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgEnumTypeWithObject_CustomRowHelper.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgObject_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueEvent_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueCustomEventization
void FDlgEvent_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDlgDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	PropertyUtils = StructCustomizationUtils.GetPropertyUtilities();

	// Cache the Property Handle for the EventType
	ParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, ParticipantName));
	EventTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, EventType));
	check(ParticipantNamePropertyHandle.IsValid());
	check(EventTypePropertyHandle.IsValid());

	// Register handler for event type change
	EventTypePropertyHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &Self::OnEventTypeChanged, true)
	);

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDlgEvent_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const bool bHasDialogue = Dialogue != nullptr;


	// Common ParticipantName
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FDlgTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDlgTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		.Update();
	}

	// EventType
	{
		EventTypePropertyRow = &StructBuilder.AddProperty(EventTypePropertyHandle.ToSharedRef());

		// Add Custom buttons
		EventTypePropertyRow_CustomDisplay = MakeShared<FDlgEnumTypeWithObject_CustomRowHelper>(
			EventTypePropertyRow,
			Dialogue,
			ParticipantNamePropertyHandle
		);
		EventTypePropertyRow_CustomDisplay->SetEnumType(EDialogueEnumWithObjectType::Event);
		EventTypePropertyRow_CustomDisplay->Update();
	}

	// EventName
	{
		const TSharedPtr<IPropertyHandle> EventNamePropertyHandle = StructPropertyHandle->GetChildHandle(
			GET_MEMBER_NAME_CHECKED(FDlgEvent, EventName)
		);
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("EventNameSearchKey", "Event Name"));

		EventNamePropertyRow = MakeShared<FDlgTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, EventNamePropertyHandle);
		EventNamePropertyRow->SetTextPropertyPickListWidget(
				SNew(SDlgTextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetDialoguesParticipantEventNames)
				.OnTextCommitted(this, &Self::HandleTextCommitted)
				.HasContextCheckbox(bHasDialogue)
				.IsContextCheckBoxChecked(false)
				.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueEventNames)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetEventNameVisibility))
		.Update();
	}

	// IntValue
	{
		IntValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, IntValue)).ToSharedRef()
		);
		IntValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetIntValueVisibility));
	}

	// FloatValue
	{
		FloatValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, FloatValue)).ToSharedRef()
		);
		FloatValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetFloatValueVisibility));
	}

	// NameValue
	{
		NameValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, NameValue)).ToSharedRef()
		);
		NameValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetNameValueVisibility));
	}

	// bDelta
	{
		BoolDeltaPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, bDelta)).ToSharedRef()
		);
		BoolDeltaPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolDeltaVisibility));
	}

	// bValue
	{
		BoolValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, bValue)).ToSharedRef()
		);
		BoolValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolValueVisibility));
	}

	// CustomEvent
	{
		CustomEventPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEvent, CustomEvent)).ToSharedRef()
		);
		CustomEventPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetCustomEventVisibility));

		// Add Custom buttons
		CustomEventPropertyRow_CustomDisplay = MakeShared<FDlgObject_CustomRowHelper>(CustomEventPropertyRow);
		CustomEventPropertyRow_CustomDisplay->Update();
		CustomEventPropertyRow_CustomDisplay->SetFunctionNameToOpen(
			EDlgBlueprintOpenType::Event,
			GET_FUNCTION_NAME_CHECKED(UDlgEventCustom, EnterEvent)
		);
	}

	// Cache the initial event type
	OnEventTypeChanged(false);
}

void FDlgEvent_Details::OnEventTypeChanged(bool bForceRefresh)
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

TArray<FName> FDlgEvent_Details::GetDialoguesParticipantEventNames() const
{
	TArray<FName> Suggestions;
	const FName ParticipantName = FDlgDetailsPanelUtils::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());

	switch (EventType)
	{
	case EDlgEventType::ModifyBool:
		Suggestions.Append(UDlgManager::GetDialoguesParticipantBoolNames(ParticipantName));
		break;

	case EDlgEventType::ModifyFloat:
		Suggestions.Append(UDlgManager::GetDialoguesParticipantFloatNames(ParticipantName));
		break;

	case EDlgEventType::ModifyInt:
		Suggestions.Append(UDlgManager::GetDialoguesParticipantIntNames(ParticipantName));
		break;

	case EDlgEventType::ModifyName:
		Suggestions.Append(UDlgManager::GetDialoguesParticipantFNameNames(ParticipantName));
		break;

	case EDlgEventType::ModifyClassIntVariable:
		if (Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FIntProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;

	case EDlgEventType::ModifyClassFloatVariable:
		if (Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FFloatProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;

	case EDlgEventType::ModifyClassBoolVariable:
		if (Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FBoolProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;

	case EDlgEventType::ModifyClassNameVariable:
		if (Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FNameProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
			FDlgHelper::SortDefault(Suggestions);
		}
		break;


	case EDlgEventType::Event:
	default:
		Suggestions.Append(UDlgManager::GetDialoguesParticipantEventNames(ParticipantName));
		break;
	}

	return Suggestions;
}

TArray<FName> FDlgEvent_Details::GetCurrentDialogueEventNames() const
{
	if (Dialogue == nullptr)
	{
		return {};
	}

	const FName ParticipantName = FDlgDetailsPanelUtils::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());
	TSet<FName> Suggestions;

	switch (EventType)
	{
	case EDlgEventType::ModifyBool:
		Suggestions.Append(Dialogue->GetParticipantBoolNames(ParticipantName));
		break;

	case EDlgEventType::ModifyName:
		Suggestions.Append(Dialogue->GetParticipantFNameNames(ParticipantName));
		break;

	case EDlgEventType::ModifyFloat:
		Suggestions.Append(Dialogue->GetParticipantFloatNames(ParticipantName));
		break;

	case EDlgEventType::ModifyInt:
		Suggestions.Append(Dialogue->GetParticipantIntNames(ParticipantName));
		break;

	case EDlgEventType::ModifyClassIntVariable:
		FNYReflectionHelper::GetVariableNames(
			Dialogue->GetParticipantClass(ParticipantName),
			FIntProperty::StaticClass(),
			Suggestions,
			GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
		);
		break;

	case EDlgEventType::ModifyClassFloatVariable:
		FNYReflectionHelper::GetVariableNames(
			Dialogue->GetParticipantClass(ParticipantName),
			FFloatProperty::StaticClass(),
			Suggestions,
			GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
		);
		break;

	case EDlgEventType::ModifyClassBoolVariable:
		FNYReflectionHelper::GetVariableNames(
			Dialogue->GetParticipantClass(ParticipantName),
			FBoolProperty::StaticClass(),
			Suggestions,
			GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
		);
		break;

	case EDlgEventType::ModifyClassNameVariable:
		FNYReflectionHelper::GetVariableNames(
			Dialogue->GetParticipantClass(ParticipantName),
			FNameProperty::StaticClass(),
			Suggestions,
			GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
		);
		break;

	case EDlgEventType::Event:
	default:
		Suggestions.Append(Dialogue->GetParticipantEventNames(ParticipantName));
		break;
	}

	FDlgHelper::SortDefault(Suggestions);
	return Suggestions.Array();
}

#undef LOCTEXT_NAMESPACE
