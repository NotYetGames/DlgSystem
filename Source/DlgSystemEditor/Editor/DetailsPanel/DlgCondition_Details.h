// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"
#include "IDetailPropertyRow.h"

#include "DlgSystem/DlgDialogue.h"
#include "DlgSystem/DlgCondition.h"
#include "DlgSystem/DlgManager.h"
#include "DlgDetailsPanelUtils.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgTextPropertyPickList_CustomRowHelper.h"

class FDlgTextPropertyPickList_CustomRowHelper;
class FDlgIntTextBox_CustomRowHelper;
class FDlgObject_CustomRowHelper;
class FDlgEnumTypeWithObject_CustomRowHelper;

/**
* How the details panel renders the FDlgCondition
* See FDlgSystemEditorModule::StartupModule for usage.
*/
class DLGSYSTEMEDITOR_API FDlgCondition_Details : public IPropertyTypeCustomization
{
	typedef FDlgCondition_Details Self;

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
		return ConditionType != EDlgConditionType::WasNodeVisited
			&& ConditionType != EDlgConditionType::HasSatisfiedChild
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetOtherParticipantNameAndVariableVisibility() const
	{
		return ConditionType != EDlgConditionType::WasNodeVisited
			&& ConditionType != EDlgConditionType::HasSatisfiedChild
			&& CompareType != EDlgCompare::ToConst
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetCallbackNameVisibility() const
	{
		return ConditionType != EDlgConditionType::WasNodeVisited
			&& ConditionType != EDlgConditionType::HasSatisfiedChild
			&& ConditionType != EDlgConditionType::Custom
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetIntValueVisibility() const
	{
		return ((CompareType == EDlgCompare::ToConst)
				&& (ConditionType == EDlgConditionType::IntCall || ConditionType == EDlgConditionType::ClassIntVariable))
				|| ConditionType == EDlgConditionType::WasNodeVisited
				|| ConditionType == EDlgConditionType::HasSatisfiedChild
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetFloatValueVisibility() const
	{
		return (CompareType == EDlgCompare::ToConst) &&
			(ConditionType == EDlgConditionType::FloatCall || ConditionType == EDlgConditionType::ClassFloatVariable)
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetNameValueVisibility() const
	{
		return(CompareType == EDlgCompare::ToConst) &&
			  (ConditionType == EDlgConditionType::NameCall || ConditionType == EDlgConditionType::ClassNameVariable)
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetBoolValueVisibility() const
	{
		return ConditionType == EDlgConditionType::EventCall
			|| ConditionType == EDlgConditionType::NameCall
			|| ConditionType == EDlgConditionType::ClassNameVariable
			|| ConditionType == EDlgConditionType::WasNodeVisited
			|| ConditionType == EDlgConditionType::HasSatisfiedChild
			|| ConditionType == EDlgConditionType::BoolCall
			|| ConditionType == EDlgConditionType::ClassBoolVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetLongTermMemoryVisibility() const
	{
		return ConditionType == EDlgConditionType::WasNodeVisited ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetGUIDVisibility() const
	{
		return ConditionType == EDlgConditionType::WasNodeVisited
			|| ConditionType == EDlgConditionType::HasSatisfiedChild
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetOperationVisibility() const
	{
		return ConditionType == EDlgConditionType::FloatCall
			|| ConditionType == EDlgConditionType::IntCall
			|| ConditionType == EDlgConditionType::ClassIntVariable
			|| ConditionType == EDlgConditionType::ClassFloatVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetCompareTypeVisibility() const
	{
		return ConditionType == EDlgConditionType::FloatCall
			|| ConditionType == EDlgConditionType::IntCall
			|| ConditionType == EDlgConditionType::BoolCall
			|| ConditionType == EDlgConditionType::NameCall
			|| ConditionType == EDlgConditionType::ClassIntVariable
			|| ConditionType == EDlgConditionType::ClassFloatVariable
			|| ConditionType == EDlgConditionType::ClassNameVariable
			|| ConditionType == EDlgConditionType::ClassBoolVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetCustomConditionVisibility() const
	{
		return ConditionType == EDlgConditionType::Custom ? EVisibility::Visible : EVisibility::Hidden;
	}

	// Gets all the condition name suggestions depending on ConditionType from all Dialogues.
	TArray<FName> GetAllDialoguesCallbackNames() const
	{
		return GetCallbackNamesForParticipant(false, false);
	}

	// Gets all the condition name suggestions depending on EventType from the current Dialogue
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

	// Gets the ParticipantNames from all Dialogues.
	TArray<FName> GetDialoguesParticipantNames() const
	{
		return UDlgManager::GetDialoguesParticipantNames();
	}

	// Gets the current Dialogue Participant Names.
	TArray<FName> GetCurrentDialogueParticipantNames() const
	{
		return FDlgDetailsPanelUtils::GetDialogueSortedParticipantNames(Dialogue);
	}

	// Handler for when text in the editable text box changed
	void HandleTextCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo) const
	{
		if (Dialogue)
		{
			Dialogue->UpdateAndRefreshData();
		}
	}

private:
	// The current Condition type of the struct.
	EDlgConditionType ConditionType = EDlgConditionType::EventCall;
	EDlgCompare CompareType = EDlgCompare::ToConst;

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
	TSharedPtr<FDlgTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FDlgTextPropertyPickList_CustomRowHelper> CallbackNamePropertyRow;
	TSharedPtr<FDlgIntTextBox_CustomRowHelper> IntValuePropertyRow;

	IDetailPropertyRow* FloatValuePropertyRow = nullptr;
	IDetailPropertyRow* NameValuePropertyRow = nullptr;
	IDetailPropertyRow* BoolValuePropertyRow = nullptr;
	IDetailPropertyRow* LongTermMemoryPropertyRow = nullptr;
	IDetailPropertyRow* OperationPropertyRow = nullptr;
	IDetailPropertyRow* CompareTypePropertyRow = nullptr;
	IDetailPropertyRow* GUIDPropertyRow = nullptr;

	IDetailPropertyRow* CustomConditionPropertyRow = nullptr;
	TSharedPtr<FDlgObject_CustomRowHelper> CustomConditionPropertyRow_CustomDisplay;

	IDetailPropertyRow* ConditionTypePropertyRow = nullptr;
	TSharedPtr<FDlgEnumTypeWithObject_CustomRowHelper> ConditionTypePropertyRow_CustomDisplay;

	TSharedPtr<FDlgTextPropertyPickList_CustomRowHelper> OtherParticipantNamePropertyRow;
	TSharedPtr<FDlgTextPropertyPickList_CustomRowHelper> OtherVariableNamePropertyRow;

	// Hold a reference to dialogue we are displaying.
	UDlgDialogue* Dialogue = nullptr;
};
