// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"
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
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

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

	// Called every time the compare type is changed
	void OnCompareTypeChanged(bool bForceRefresh);

	// Getters for the visibility of some properties
	EVisibility GetParticipantNameVisibility() const
	{
		return ConditionType != EDlgConditionType::DlgConditionNodeVisited
			&& ConditionType != EDlgConditionType::DlgConditionHasSatisfiedChild
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetOtherParticipantNameAndVariableVisibility() const
	{
		return ConditionType != EDlgConditionType::DlgConditionNodeVisited
			&& ConditionType != EDlgConditionType::DlgConditionHasSatisfiedChild
			&& CompareType != EDlgCompareType::DlgCompareToConst
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetCallbackNameVisibility() const
	{
		return ConditionType != EDlgConditionType::DlgConditionNodeVisited
			&& ConditionType != EDlgConditionType::DlgConditionHasSatisfiedChild
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetIntValueVisibility() const
	{
		return ((CompareType == EDlgCompareType::DlgCompareToConst)
				&& (ConditionType == EDlgConditionType::DlgConditionIntCall || ConditionType == EDlgConditionType::DlgConditionClassIntVariable))
				|| ConditionType == EDlgConditionType::DlgConditionNodeVisited
				|| ConditionType == EDlgConditionType::DlgConditionHasSatisfiedChild
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetFloatValueVisibility() const
	{
		return (CompareType == EDlgCompareType::DlgCompareToConst) &&
			(ConditionType == EDlgConditionType::DlgConditionFloatCall || ConditionType == EDlgConditionType::DlgConditionClassFloatVariable)
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetNameValueVisibility() const
	{
		return(CompareType == EDlgCompareType::DlgCompareToConst) &&
			  (ConditionType == EDlgConditionType::DlgConditionNameCall || ConditionType == EDlgConditionType::DlgConditionClassNameVariable)
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetBoolValueVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionEventCall
			|| ConditionType == EDlgConditionType::DlgConditionNameCall
			|| ConditionType == EDlgConditionType::DlgConditionClassNameVariable
			|| ConditionType == EDlgConditionType::DlgConditionNodeVisited
			|| ConditionType == EDlgConditionType::DlgConditionHasSatisfiedChild
			|| ConditionType == EDlgConditionType::DlgConditionBoolCall
			|| ConditionType == EDlgConditionType::DlgConditionClassBoolVariable

			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetLongTermMemoryVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionNodeVisited ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetOperationVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionFloatCall
			|| ConditionType == EDlgConditionType::DlgConditionIntCall
			|| ConditionType == EDlgConditionType::DlgConditionClassIntVariable
			|| ConditionType == EDlgConditionType::DlgConditionClassFloatVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetCompareTypeVisibility() const
	{
		return ConditionType == EDlgConditionType::DlgConditionFloatCall
			|| ConditionType == EDlgConditionType::DlgConditionIntCall
			|| ConditionType == EDlgConditionType::DlgConditionBoolCall
			|| ConditionType == EDlgConditionType::DlgConditionNameCall
			|| ConditionType == EDlgConditionType::DlgConditionClassIntVariable
			|| ConditionType == EDlgConditionType::DlgConditionClassFloatVariable
			|| ConditionType == EDlgConditionType::DlgConditionClassNameVariable
			|| ConditionType == EDlgConditionType::DlgConditionClassBoolVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	/** Gets all the condition name suggestions depending on ConditionType from all Dialogues. */
	TArray<FName> GetAllDialoguesCallbackNames() const
	{
		return GetCallbackNamesForParticipant(false, false);
	}

	/** Gets all the condition name suggestions depending on EventType from the current Dialogue */
	TArray<FName> GetCurrentDialogueCallbackNames() const
	{
		return GetCallbackNamesForParticipant(true, false);
	}

	TArray<FName> GetAllDialoguesOtherVariableNames() const
	{
		return GetCallbackNamesForParticipant(false, true);
	}

	TArray<FName> GetCurrentDialogueOtherVariableNames() const
	{
		return GetCallbackNamesForParticipant(true, true);
	}

	TArray<FName> GetCallbackNamesForParticipant(bool bCurrentOnly, bool bOtherValue) const;

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
		return FDialogueDetailsPanelUtils::GetDialogueSortedParticipantNames(Dialogue);
	}

	/** Handler for when text in the editable text box changed */
	void HandleTextCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo) const
	{
		if (Dialogue)
		{
			Dialogue->RefreshData();
		}
	}

private:
	// The current Condition type of the struct.
	EDlgConditionType ConditionType = EDlgConditionType::DlgConditionEventCall;
	EDlgCompareType CompareType = EDlgCompareType::DlgCompareToConst;

	// Cache the Struct property handle
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	// Cache the property handle of some properties
	TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle;
	TSharedPtr<IPropertyHandle> OtherParticipantNamePropertyHandle;
	TSharedPtr<IPropertyHandle> ConditionTypePropertyHandle;
	TSharedPtr<IPropertyHandle> CompareTypePropertyHandle;
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
	IDetailPropertyRow* CompareTypePropertyRow = nullptr;
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> OtherParticipantNamePropertyRow;
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> OtherVariableNamePropertyRow;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;
};
