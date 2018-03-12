// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Visibility.h"
#include "IDetailPropertyRow.h"

#include "DlgDialogue.h"
#include "DlgCondition.h"
#include "DlgManager.h"
#include "DialogueDetailsPanelUtils.h"
#include "CustomRowHelpers/TextPropertyPickList_CustomRowHelper.h"

class FTextPropertyPickList_CustomRowHelper;

/**
* How the details panel renders the FDlgCondition
* See FDlgSystemEditorModule::StartupModule for usage.
*/
class FDialogueCondition_Details : public IPropertyTypeCustomization
{
	typedef FDialogueCondition_Details Self;

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new Self); }

	/** IPropertyTypeCustomization interface */
	/**
	 * Called when the header of the property (the row in the details panel where the property is shown)
	 * If nothing is added to the row, the header is not displayed
	 *
	 * @param StructPropertyHandle		Handle to the property being customized
	 * @param HeaderRow					A row that widgets can be added to
	 * @param StructCustomizationUtils	Utilities for customization
	 */
	void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
		FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	/**
	 * Called when the children of the property should be customized or extra rows added
	 *
	 * @param StructPropertyHandle		Handle to the property being customized
	 * @param StructBuilder				A builder for adding children
	 * @param StructCustomizationUtils	Utilities for customization
	 */
	void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
		IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	// Called every time the condition type is changed
	void OnConditionTypeChanged(bool bForceRefresh);

	// Getters for the visibility of some properties
	EVisibility GetParticipantNameVisibility() const
	{
		return (ConditionType != EDlgConditionType::DlgConditionNodeVisited && ConditionType != EDlgConditionType::DlgConditionHasSatisfiedChild)
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetCallbackNameVisibility() const
	{
		return (ConditionType != EDlgConditionType::DlgConditionNodeVisited && ConditionType != EDlgConditionType::DlgConditionHasSatisfiedChild)
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetIntValueVisibility() const
	{
		return (ConditionType == EDlgConditionType::DlgConditionIntCall || 
			    ConditionType == EDlgConditionType::DlgConditionNodeVisited ||
			    ConditionType == EDlgConditionType::DlgConditionHasSatisfiedChild)
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetFloatValueVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionFloatCall ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetNameValueVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionNameCall ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetBoolValueVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionEventCall || ConditionType == EDlgConditionType::DlgConditionBoolCall
			|| ConditionType == EDlgConditionType::DlgConditionNodeVisited || ConditionType == EDlgConditionType::DlgConditionNameCall
			|| ConditionType == EDlgConditionType::DlgConditionHasSatisfiedChild ?
			EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetLongTermMemoryVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionNodeVisited ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetOperationVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionFloatCall || ConditionType == EDlgConditionType::DlgConditionIntCall
			? EVisibility::Visible : EVisibility::Hidden;
	}

	/** Gets all the condition name suggestions depending on ConditionType from all Dialogues. */
	TArray<FName> GetAllDialoguesCallbackNames() const
	{
		TArray<FName> Suggestions;
		const FName ParticipantName = DetailsPanel::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());

		switch (ConditionType)
		{
		case EDlgConditionType::DlgConditionBoolCall:
			UDlgManager::GetAllDialoguesBoolNames(ParticipantName, Suggestions);
			break;

		case EDlgConditionType::DlgConditionFloatCall:
			UDlgManager::GetAllDialoguesFloatNames(ParticipantName, Suggestions);
			break;

		case EDlgConditionType::DlgConditionIntCall:
			UDlgManager::GetAllDialoguesIntNames(ParticipantName, Suggestions);
			break;

		case EDlgConditionType::DlgConditionNameCall:
			UDlgManager::GetAllDialoguesNameNames(ParticipantName, Suggestions);
			break;

		case EDlgConditionType::DlgConditionEventCall:
		case EDlgConditionType::DlgConditionNodeVisited:
		default:
			UDlgManager::GetAllDialoguesConditionNames(ParticipantName, Suggestions);
			break;
		}

		return Suggestions;
	}

	/** Gets all the condition name suggestions depending on EventType from the current Dialogue */
	TArray<FName> GetCurrentDialogueCallbackNames() const
	{
		const FName ParticipantName = DetailsPanel::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());
		TSet<FName> Names;

		switch (ConditionType)
		{
		case EDlgConditionType::DlgConditionBoolCall:
			Dialogue->GetBoolNames(ParticipantName, Names);
			break;

		case EDlgConditionType::DlgConditionNameCall:
			Dialogue->GetNameNames(ParticipantName, Names);
			break;

		case EDlgConditionType::DlgConditionFloatCall:
			Dialogue->GetFloatNames(ParticipantName, Names);
			break;

		case EDlgConditionType::DlgConditionIntCall:
			Dialogue->GetIntNames(ParticipantName, Names);
			break;

		case EDlgConditionType::DlgConditionEventCall:
		case EDlgConditionType::DlgConditionNodeVisited:
		default:
			Dialogue->GetConditions(ParticipantName, Names);
			break;
		}

		UDlgManager::SortDefault(Names);
		return Names.Array();
	}

	/** Gets the ParticipantNames from all Dialogues. */
	TArray<FName> GetAllDialoguesParticipantNames() const
	{
		TArray<FName> OutArray;
		UDlgManager::GetAllDialoguesParticipantNames(OutArray);
		return OutArray;
	}

	/** Gets the current Dialogue Participant Names.  */
	TArray<FName> GetCurrentDialogueParticipantNames() const
	{
		return DetailsPanel::GetDialogueSortedParticipantNames(Dialogue);
	}

	/** Handler for when text in the editable text box changed */
	void HandleTextCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo) const
	{
		Dialogue->RefreshData();
	}

private:
	// The current Condition type of the struct.
	EDlgConditionType ConditionType = EDlgConditionType::DlgConditionEventCall;

	// Cache the Struct property handle
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	// Cache the property handle of some properties
	TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle;
	TSharedPtr<IPropertyHandle> ConditionTypePropertyHandle;
	TSharedPtr<IPropertyHandle> IntValuePropertyHandle;

	// just some nice utilities
	TSharedPtr<IPropertyUtilities> PropertyUtils;

	// Cache the rows of the properties, created in CustomizeChildren
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> CallbackNamePropertyRow;
	IDetailPropertyRow* IntValuePropertyRow = nullptr;
	IDetailPropertyRow* FloatValuePropertyRow = nullptr;
	IDetailPropertyRow* NameValuePropertyRow = nullptr;
	IDetailPropertyRow* BoolValuePropertyRow = nullptr;
	IDetailPropertyRow* LongTermMemoryPropertyRow = nullptr;
	IDetailPropertyRow* OperationPropertyRow = nullptr;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;
};
