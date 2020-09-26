// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueCondition_Details.h"

#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"

#include "Nodes/DlgNode.h"
#include "NYReflectionHelper.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "Widgets/SDialogueTextPropertyPickList.h"
#include "IPropertyUtilities.h"
#include "Widgets/DialogueTextPropertyPickList_CustomRowHelper.h"
#include "DlgHelper.h"
#include "Widgets/DialogueEnumTypeWithObject_CustomRowHelper.h"
#include "Widgets/DialogueIntTextBox_CustomRowHelper.h"
#include "Widgets/DialogueObject_CustomRowHelper.h"

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

void FDialogueCondition_Details::CustomizeChildren(
	TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils
)
{
	const bool bHasDialogue = Dialogue != nullptr;

	// Add common ConditionStrength, ConditionType
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, Strength)).ToSharedRef());

	// ConditionType
	{
		ConditionTypePropertyRow = &StructBuilder.AddProperty(ConditionTypePropertyHandle.ToSharedRef());

		// Add Custom buttons
		ConditionTypePropertyRow_CustomDisplay = MakeShared<FDialogueEnumTypeWithObject_CustomRowHelper>(
			ConditionTypePropertyRow,
			Dialogue,
			ParticipantNamePropertyHandle
		);
		ConditionTypePropertyRow_CustomDisplay->SetEnumType(EDialogueEnumWithObjectType::Condition);
		ConditionTypePropertyRow_CustomDisplay->Update();
	}

	// ParticipantName
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.IsEnabled(InStructPropertyHandle->IsEditable())
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetParticipantNameVisibility))
		.Update();
	}

	// CallbackName (variable name)
	{
		const TSharedPtr<IPropertyHandle> CallbackNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, CallbackName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("CallBackNameSearchKey", "Variable Name"));

		CallbackNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, CallbackNamePropertyHandle);
		CallbackNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.IsEnabled(InStructPropertyHandle->IsEditable())
			.AvailableSuggestions(this, &Self::GetAllDialoguesCallbackNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(false)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueCallbackNames)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetCallbackNameVisibility))
		.Update();
	}

	// Operation
	{
		OperationPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, Operation)).ToSharedRef()
		);
		OperationPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetOperationVisibility));
	}

	// CompareType
	{
		CompareTypePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, CompareType)).ToSharedRef()
		);
		CompareTypePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetCompareTypeVisibility));
	}

	// OtherParticipantName
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, OtherParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.IsEnabled(InStructPropertyHandle->IsEditable())
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(true)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetOtherParticipantNameAndVariableVisibility))
		.Update();
	}

	// Other variable name
	{
		const TSharedPtr<IPropertyHandle> CallbackNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, OtherVariableName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("CallBackNameSearchKey", "Variable Name"));

		OtherVariableNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, CallbackNamePropertyHandle);
		OtherVariableNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.IsEnabled(InStructPropertyHandle->IsEditable())
			.AvailableSuggestions(this, &Self::GetAllDialoguesOtherVariableNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(true)
			.IsContextCheckBoxChecked(false)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueOtherVariableNames)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetOtherParticipantNameAndVariableVisibility))
		.Update();
	}


	// IntValue
	{
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("IntValueSearchKey", "Int Value"));
		IntValuePropertyRow = MakeShared<FDialogueIntTextBox_CustomRowHelper>(DetailWidgetRow, IntValuePropertyHandle, Dialogue);
		IntValuePropertyRow->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetIntValueVisibility));
		IntValuePropertyRow->SetJumpToNodeVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetGUIDVisibility));
		IntValuePropertyRow->Update();
	}

	// FloatValue
	{
		FloatValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, FloatValue)).ToSharedRef()
		);
		FloatValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetFloatValueVisibility));
	}

	// NameValue
	{
		NameValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, NameValue)).ToSharedRef()
		);
		NameValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetNameValueVisibility));
	}

	// bBoolValue
	{
		BoolValuePropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, bBoolValue)).ToSharedRef()
		);
		BoolValuePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetBoolValueVisibility));
	}

	// bLongTermMemory
	{
		LongTermMemoryPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, bLongTermMemory)).ToSharedRef()
		);
		LongTermMemoryPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetLongTermMemoryVisibility));
	}

	// GUID
	{
		GUIDPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, GUID)).ToSharedRef()
		);
		GUIDPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetGUIDVisibility));
	}

	// CustomCondition
	{
		CustomConditionPropertyRow = &StructBuilder.AddProperty(
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgCondition, CustomCondition)).ToSharedRef()
		);
		CustomConditionPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetCustomConditionVisibility));

		// Add Custom buttons
		CustomConditionPropertyRow_CustomDisplay = MakeShared<FDialogueObject_CustomRowHelper>(CustomConditionPropertyRow);
		CustomConditionPropertyRow_CustomDisplay->Update();
		CustomConditionPropertyRow_CustomDisplay->SetFunctionNameToOpen(
			EDialogueBlueprintOpenType::Function,
			GET_FUNCTION_NAME_CHECKED(UDlgConditionCustom, IsConditionMet)
		);
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
	FText CallBackNameDisplayName = LOCTEXT("CallBackNameDisplayName", "Variable Name");
	FText CallBackNameToolTip = LOCTEXT("CallBackNameToolTip", "The name of the checked variable");
	FText BoolValueDisplayName = LOCTEXT("BoolValueDisplayName", "Return Value");
	FText BoolValueToolTip = LOCTEXT("BoolValueToolTip", "SHOULD NOT BE VISIBLE");
	// TODO remove the "equal" operations for float values as they are imprecise
	FText FloatValueToolTip = LOCTEXT(
		"FloatValueToolTip",
		"The float value the VariableName is checked against (depending on the operation).\n"
		"VariableName <Operation> FloatValue"
	);
	FText IntValueDisplayName = LOCTEXT("IntValueDisplayName", "Int Value");
	FText IntValueToolTip = LOCTEXT(
		"IntValueToolTip",
		"The int value the VariableName is checked against (depending on the operation).\n"
		"VariableName <Operation> IntValue"
	);
	FText GUIDDisplayName = LOCTEXT("GUIDDisplayName", "Node GUID");
	FText GUIDToolTip = LOCTEXT(
		"GUIDToolTip",
		"The Corresponding GUID of the Node Index. (Set On Compile)"
	);

	FDialogueDetailsPanelUtils::ResetNumericPropertyLimits(IntValuePropertyHandle);
	switch (ConditionType)
	{
	case EDlgConditionType::EventCall:
		CallBackNameDisplayName = LOCTEXT("ConditionEvent_CallBackNameDisplayName", "Condition Name");
		CallBackNameToolTip = LOCTEXT("ConditionEvent_CallBackNameToolTip", "Name parameter of the event call the participant gets");
		BoolValueToolTip = LOCTEXT("ConditionEvent_BoolValueToolTip", "Does the return result of the Event/Condition has this boolean value?");
		break;

	case EDlgConditionType::BoolCall:
	case EDlgConditionType::ClassBoolVariable:
		BoolValueToolTip = LOCTEXT("ConditionBool_BoolValueToolTip", "Whether the bool check is expected to be true or false in order to satisfy the condition");
		break;

	case EDlgConditionType::NameCall:
	case EDlgConditionType::ClassNameVariable:
		BoolValueToolTip = LOCTEXT("ConditionBool_BoolValueToolTip", "Whether the two Name is expected to be equal or not in order to satisfy the condition");
		BoolValueDisplayName = LOCTEXT("BoolValueDisplayName", "Succeed on Equal");
		break;

	case EDlgConditionType::FloatCall:
	case EDlgConditionType::ClassFloatVariable:
		break;

	case EDlgConditionType::IntCall:
	case EDlgConditionType::ClassIntVariable:
		break;

	case EDlgConditionType::WasNodeVisited:
		if (Dialogue)
		{
			FDialogueDetailsPanelUtils::SetNumericPropertyLimits<int32>(IntValuePropertyHandle, 0, Dialogue->GetNodes().Num() - 1);
		}

		IntValueDisplayName = LOCTEXT("ConditionNodeVisited_IntValueDisplayName", "Node Index");
		IntValueToolTip = LOCTEXT("ConditionNodeVisited_IntValueToolTip", "Node index of the node we want to check the visited status");
		BoolValueDisplayName = LOCTEXT("ConditionNodeVisited_BoolValueDisplayName", "Is Node Visited?");
		BoolValueToolTip = LOCTEXT("ConditionNodeVisited_BoolValueToolTip", "Should the node be visited? True/False.");
		break;

	case EDlgConditionType::HasSatisfiedChild:
		if (Dialogue)
		{
			FDialogueDetailsPanelUtils::SetNumericPropertyLimits<int32>(IntValuePropertyHandle, 0, Dialogue->GetNodes().Num() - 1);
		}

		IntValueDisplayName = LOCTEXT("ConditionHasSatisfiedChild_IntValueDisplayName", "Node Index");
		IntValueToolTip = LOCTEXT("ConditionHasSatisfiedChild_IntValueToolTip", "Node index of the node we want to check");
		BoolValueDisplayName = LOCTEXT("ConditionHasSatisfiedChild_BoolValueDisplayName", "Has?");
		BoolValueToolTip = LOCTEXT("ConditionHasSatisfiedChild_BoolValueToolTip", "Should the node have satisfied child(ren)?");
		break;

	case EDlgConditionType::Custom:
		break;

	default:
		checkNoEntry();
	}

	CallbackNamePropertyRow->SetDisplayName(CallBackNameDisplayName)
		.SetToolTip(CallBackNameToolTip)
		.Update();

	BoolValuePropertyRow->DisplayName(BoolValueDisplayName);
	BoolValuePropertyRow->ToolTip(BoolValueToolTip);

	IntValuePropertyRow->SetDisplayName(IntValueDisplayName);
	IntValuePropertyRow->SetToolTip(IntValueToolTip);
	IntValuePropertyRow->Update();
	FloatValuePropertyRow->ToolTip(FloatValueToolTip);

	GUIDPropertyRow->DisplayName(GUIDDisplayName);
	GUIDPropertyRow->ToolTip(GUIDToolTip);

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
	CompareType = static_cast<EDlgCompare>(Value);

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
		bReflectionBased = CompareType == EDlgCompare::ToClassVariable;
	}
	else
	{
		bReflectionBased = ConditionType == EDlgConditionType::ClassBoolVariable
						|| ConditionType == EDlgConditionType::ClassIntVariable
						|| ConditionType == EDlgConditionType::ClassFloatVariable
						|| ConditionType == EDlgConditionType::ClassNameVariable;
	}

	switch (ConditionType)
	{
	case EDlgConditionType::ClassBoolVariable:
	case EDlgConditionType::BoolCall:
		if (bReflectionBased && Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FNYBoolProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
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

	case EDlgConditionType::ClassFloatVariable:
	case EDlgConditionType::FloatCall:
		if (bReflectionBased && Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FNYFloatProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
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

	case EDlgConditionType::ClassIntVariable:
	case EDlgConditionType::IntCall:
		if (bReflectionBased && Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FNYIntProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
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

	case EDlgConditionType::NameCall:
	case EDlgConditionType::ClassNameVariable:
		if (bReflectionBased && Dialogue)
		{
			FNYReflectionHelper::GetVariableNames(
				Dialogue->GetParticipantClass(ParticipantName),
				FNYNameProperty::StaticClass(),
				Suggestions,
				GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses
			);
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

	case EDlgConditionType::EventCall:
	case EDlgConditionType::WasNodeVisited:
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
