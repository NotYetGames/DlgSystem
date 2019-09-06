// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueCondition_Details.h"

#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"

#include "Nodes/DlgNode.h"
#include "DlgReflectionHelper.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "STextPropertyPickList.h"
#include "IPropertyUtilities.h"
#include "CustomRowHelpers/TextPropertyPickList_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueCondition_Details"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueCondition_Details
void FDialogueCondition_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDialogueDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	PropertyUtils = StructCustomizationUtils.GetPropertyUtilities();

	// Cache the Property Handle for some properties
	ParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, ParticipantName));
	OtherParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, OtherParticipantName));
	ConditionTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, ConditionType));
	CompareTypePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, CompareType));
	IntValuePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, IntValue));
	check(ParticipantNamePropertyHandle.IsValid());
	check(OtherParticipantNamePropertyHandle.IsValid());
	check(ConditionTypePropertyHandle.IsValid());
	check(CompareTypePropertyHandle.IsValid());
	check(IntValuePropertyHandle.IsValid());

	// Register handler properties changes
	ConditionTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &Self::OnConditionTypeChanged, true));

	CompareTypePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &Self::OnCompareTypeChanged, true));

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDialogueCondition_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const bool bHasDialogue = Dialogue != nullptr;

	// Add common ConditionStrength, ConditionType
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, Strength)).ToSharedRef());
	StructBuilder.AddProperty(ConditionTypePropertyHandle.ToSharedRef());

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
		->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetParticipantNameVisibility))
		->Update();
	}

	// CallbackName (variable name)
	{
		const TSharedPtr<IPropertyHandle> CallbackNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, CallbackName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("CallBackNameSearchKey", "Variable Name"));

		CallbackNamePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, CallbackNamePropertyHandle);
		CallbackNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesCallbackNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(false)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueCallbackNames)
		)
		->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetCallbackNameVisibility))
		->Update();
	}

	// Operation
	{
		OperationPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, Operation)).ToSharedRef());
		OperationPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetOperationVisibility));
	}

	// CompareType
	{
		CompareTypePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, CompareType)).ToSharedRef());
		CompareTypePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetCompareTypeVisibility));
	}


	// OtherParticipantName
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, OtherParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(true)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetOtherParticipantNameAndVariableVisibility))
		->Update();
	}

	// Other variable name
	{
		const TSharedPtr<IPropertyHandle> CallbackNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, OtherVariableName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("CallBackNameSearchKey", "Variable Name"));

		OtherVariableNamePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, CallbackNamePropertyHandle);
		OtherVariableNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesOtherVariableNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(true)
			.IsContextCheckBoxChecked(false)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueOtherVariableNames)
		)
		->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetOtherParticipantNameAndVariableVisibility))
		->Update();
	}


	// IntValue
	{
		IntValuePropertyRow = &StructBuilder.AddProperty(IntValuePropertyHandle.ToSharedRef());
		IntValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetIntValueVisibility));
	}

	// FloatValue
	{
		FloatValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, FloatValue)).ToSharedRef());
		FloatValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetFloatValueVisibility));
	}

	// NameValue
	{
		NameValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, NameValue)).ToSharedRef());
		NameValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetNameValueVisibility));
	}

	// bBoolValue
	{
		BoolValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, bBoolValue)).ToSharedRef());
		BoolValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolValueVisibility));
	}

	// bLongTermMemory
	{
		LongTermMemoryPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, bLongTermMemory)).ToSharedRef());
		LongTermMemoryPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetLongTermMemoryVisibility));
	}

	// Cache the initial values
	OnCompareTypeChanged(false);
	OnConditionTypeChanged(false);
}

void FDialogueCondition_Details::OnConditionTypeChanged(bool bForceRefresh)
{
	// Update to the new type
	uint8 Value = 0;
	if (ConditionTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}
	ConditionType = static_cast<EDlgConditionType>(Value);

	// Update the display names/tooltips
	FText CalllBackNameDisplayName = LOCTEXT("CalllBackNameDisplayName", "Variable Name");
	FText CalllBackNameToolTip = LOCTEXT("CalllBackNameToolTip", "The name of the checked variable");
	FText BoolValueDisplayName = LOCTEXT("BoolValueDisplayName", "Return Value");
	FText BoolValueToolTip = LOCTEXT("BoolValueToolTip", "SHOULD NOT BE VISIBLE");
	// TODO remove the "equal" operations for float values as they are imprecise
	FText FloatValueToolTip = LOCTEXT("FloatValueToolTip", "The float value the VariableName is checked against (depending on the operation).\n"
		"VariableName <Operation> FloatValue");
	FText IntValueDisplayName = LOCTEXT("IntValueDisplayName", "Int Value");
	FText IntValueToolTip = LOCTEXT("IntValueToolTip", "The int value the VariableName is checked against (depending on the operation).\n"
		"VariableName <Operation> IntValue");

	FDialogueDetailsPanelUtils::ResetNumericPropertyLimits(IntValuePropertyHandle);
	switch (ConditionType)
	{
	case EDlgConditionType::DlgConditionEventCall:
		CalllBackNameDisplayName = LOCTEXT("ConditionEvent_CallBackNameDisplayName", "Condition Name");
		CalllBackNameToolTip = LOCTEXT("ConditionEvent_CallBackNameToolTip", "Name parameter of the event call the participant gets");
		BoolValueToolTip = LOCTEXT("ConditionEvent_BoolValueToolTip", "Does the return result of the Event/Condition has this boolean value?");
		break;

	case EDlgConditionType::DlgConditionBoolCall:
	case EDlgConditionType::DlgConditionClassBoolVariable:
		BoolValueToolTip = LOCTEXT("ConditionBool_BoolValueToolTip", "Whether the bool check is expected to be true or false in order to satisfy the condition");
		break;

	case EDlgConditionType::DlgConditionNameCall:
	case EDlgConditionType::DlgConditionClassNameVariable:
		BoolValueToolTip = LOCTEXT("ConditionBool_BoolValueToolTip", "Whether the two Name is expected to be equal or not in order to satisfy the condition");
		BoolValueDisplayName = LOCTEXT("BoolValueDisplayName", "Succeed on Equal");
		break;

	case EDlgConditionType::DlgConditionFloatCall:
	case EDlgConditionType::DlgConditionClassFloatVariable:
		break;

	case EDlgConditionType::DlgConditionIntCall:
	case EDlgConditionType::DlgConditionClassIntVariable:
		break;

	case EDlgConditionType::DlgConditionNodeVisited:
		if (Dialogue)
		{
			FDialogueDetailsPanelUtils::SetNumericPropertyLimits<int32>(IntValuePropertyHandle, 0, Dialogue->GetNodes().Num() - 1);
		}

		IntValueDisplayName = LOCTEXT("ConditionNodeVisited_IntValueDisplayName", "Node Index");
		IntValueToolTip = LOCTEXT("ConditionNodeVisited_IntValueToolTip", "Node index of the node we want to check the visited status");
		BoolValueDisplayName = LOCTEXT("ConditionNodeVisited_BoolValueDisplayName", "Is Node Visited?");
		BoolValueToolTip = LOCTEXT("ConditionNodeVisited_BoolValueToolTip", "Should the node be visited? True/False.");
		break;

	case EDlgConditionType::DlgConditionHasSatisfiedChild:
		if (Dialogue)
		{
			FDialogueDetailsPanelUtils::SetNumericPropertyLimits<int32>(IntValuePropertyHandle, 0, Dialogue->GetNodes().Num() - 1);
		}

		IntValueDisplayName = LOCTEXT("ConditionHasSatisfiedChild_IntValueDisplayName", "Node Index");
		IntValueToolTip = LOCTEXT("ConditionHasSatisfiedChild_IntValueToolTip", "Node index of the node we want to check");
		BoolValueDisplayName = LOCTEXT("ConditionHasSatisfiedChild_BoolValueDisplayName", "Has?");
		BoolValueToolTip = LOCTEXT("ConditionHasSatisfiedChild_BoolValueToolTip", "Should the node have satisfied child(ren)?");
		break;

	default:
		checkNoEntry();
	}

	CallbackNamePropertyRow->SetDisplayName(CalllBackNameDisplayName)
		->SetToolTip(CalllBackNameToolTip)
		->Update();

	BoolValuePropertyRow->DisplayName(BoolValueDisplayName);
	BoolValuePropertyRow->ToolTip(BoolValueToolTip);

	IntValuePropertyRow->DisplayName(IntValueDisplayName);
	IntValuePropertyRow->ToolTip(IntValueToolTip);
	FloatValuePropertyRow->ToolTip(FloatValueToolTip);

	// Refresh the view, without this some names/tooltips won't get refreshed
	if (bForceRefresh && PropertyUtils.IsValid())
	{
		PropertyUtils->ForceRefresh();
	}
}

void FDialogueCondition_Details::OnCompareTypeChanged(bool bForceRefresh)
{
	uint8 Value = 0;
	if (CompareTypePropertyHandle->GetValue(Value) != FPropertyAccess::Success)
	{
		return;
	}
	CompareType = static_cast<EDlgCompareType>(Value);

	// Refresh the view, without this some names/tooltips won't get refreshed
	if (bForceRefresh && PropertyUtils.IsValid())
	{
		PropertyUtils->ForceRefresh();
	}
}

/** Gets all the condition name suggestions depending on ConditionType from all Dialogues. */
TArray<FName> FDialogueCondition_Details::GetCallbackNamesForParticipant(bool bCurrentOnly, bool bOtherValue) const
{
	TArray<FName> Suggestions;
	TSet<FName> SuggestionSet;
	const TSharedPtr<IPropertyHandle>& ParticipantHandle = bOtherValue ? OtherParticipantNamePropertyHandle : ParticipantNamePropertyHandle;
	const FName ParticipantName = FDialogueDetailsPanelUtils::GetParticipantNameFromPropertyHandle(ParticipantHandle.ToSharedRef());

	bool bReflectionBased = false;
	if (bOtherValue)
	{
		bReflectionBased = CompareType == EDlgCompareType::DlgCompareToClassVariable;
	}
	else
	{
		bReflectionBased = ConditionType == EDlgConditionType::DlgConditionClassBoolVariable
						|| ConditionType == EDlgConditionType::DlgConditionClassIntVariable
						|| ConditionType == EDlgConditionType::DlgConditionClassFloatVariable
						|| ConditionType == EDlgConditionType::DlgConditionClassNameVariable;
	}


	switch (ConditionType)
	{
	case EDlgConditionType::DlgConditionClassBoolVariable:
	case EDlgConditionType::DlgConditionBoolCall:
		if (bReflectionBased && Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UBoolProperty::StaticClass(), Suggestions);
		}
		else
		{
			if (bCurrentOnly && Dialogue)
			{
				Dialogue->GetBoolNames(ParticipantName, SuggestionSet);
				Suggestions = SuggestionSet.Array();
			}
			else
			{
				UDlgManager::GetAllDialoguesBoolNames(ParticipantName, Suggestions);
			}
		}
		break;

	case EDlgConditionType::DlgConditionClassFloatVariable:
	case EDlgConditionType::DlgConditionFloatCall:
		if (bReflectionBased && Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UFloatProperty::StaticClass(), Suggestions);
		}
		else
		{
			if (bCurrentOnly && Dialogue)
			{
				Dialogue->GetFloatNames(ParticipantName, SuggestionSet);
			}
			else
			{
				UDlgManager::GetAllDialoguesFloatNames(ParticipantName, Suggestions);
			}
		}
		break;

	case EDlgConditionType::DlgConditionClassIntVariable:
	case EDlgConditionType::DlgConditionIntCall:
		if (bReflectionBased && Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UIntProperty::StaticClass(), Suggestions);
		}
		else
		{
			if (bCurrentOnly && Dialogue)
			{
				Dialogue->GetIntNames(ParticipantName, SuggestionSet);
			}
			else
			{
				UDlgManager::GetAllDialoguesIntNames(ParticipantName, Suggestions);
			}
		}
		break;

	case EDlgConditionType::DlgConditionNameCall:
	case EDlgConditionType::DlgConditionClassNameVariable:
		if (bReflectionBased && Dialogue)
		{
			UDlgReflectionHelper::GetVariableNames(Dialogue->GetParticipantClass(ParticipantName), UNameProperty::StaticClass(), Suggestions);
		}
		else
		{
			if (bCurrentOnly && Dialogue)
			{
				Dialogue->GetNameNames(ParticipantName, SuggestionSet);
			}
			else
			{
				UDlgManager::GetAllDialoguesNameNames(ParticipantName, Suggestions);
			}
		}
		break;

	case EDlgConditionType::DlgConditionEventCall:
	case EDlgConditionType::DlgConditionNodeVisited:
	default:
		if (bCurrentOnly && Dialogue)
		{
			Dialogue->GetConditions(ParticipantName, SuggestionSet);
		}
		else
		{
			UDlgManager::GetAllDialoguesConditionNames(ParticipantName, Suggestions);
		}
		break;
	}

	if (SuggestionSet.Num() > Suggestions.Num())
	{
		Suggestions = SuggestionSet.Array();
	}

	FDlgHelper::SortDefault(Suggestions);
	return Suggestions;
}


#undef LOCTEXT_NAMESPACE
