// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"
#include "IDetailPropertyRow.h"

#include "DlgEvent.h"
#include "DlgManager.h"
#include "DialogueDetailsPanelUtils.h"

class FDialogueTextPropertyPickList_CustomRowHelper;
class FDialogueObject_CustomRowHelper;
class FDialogueEnumTypeWithObject_CustomRowHelper;

/**
 * How the details panel renders the FDlgEvent
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class FDialogueEvent_Details : public IPropertyTypeCustomization
{
	typedef FDialogueEvent_Details Self;

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
	// Called every time the event type is changed
	void OnEventTypeChanged(bool bForceRefresh);

	// Getters for the visibility of some properties
	EVisibility GetEventNameVisibility() const
	{
		return EventType != EDlgEventType::Custom
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetIntValueVisibility() const
	{
		return EventType == EDlgEventType::ModifyInt
			|| EventType == EDlgEventType::ModifyClassIntVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetFloatValueVisibility() const
	{
		return EventType == EDlgEventType::ModifyFloat
			|| EventType == EDlgEventType::ModifyClassFloatVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetNameValueVisibility() const
	{
		return EventType == EDlgEventType::ModifyName
			|| EventType == EDlgEventType::ModifyClassNameVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetBoolDeltaVisibility() const
	{
		return EventType == EDlgEventType::ModifyInt
			|| EventType == EDlgEventType::ModifyFloat
			|| EventType == EDlgEventType::ModifyClassIntVariable
			|| EventType == EDlgEventType::ModifyClassFloatVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetBoolValueVisibility() const
	{
		return EventType == EDlgEventType::ModifyBool
			|| EventType == EDlgEventType::ModifyClassBoolVariable
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetCustomEventVisibility() const
	{
		return EventType == EDlgEventType::Custom ? EVisibility::Visible : EVisibility::Hidden;
	}

	// Gets all the event name suggestions depending on EventType from all Dialogues.
	TArray<FName> GetAllDialoguesEventNames() const;

	// Gets all the event name suggestions depending on EventType from the current Dialogue
	TArray<FName> GetCurrentDialogueEventNames() const;

	// Gets the ParticipantNames from all Dialogues.
	TArray<FName> GetAllDialoguesParticipantNames() const
	{
		TArray<FName> OutArray;
		UDlgManager::GetAllDialoguesParticipantNames(OutArray);
		return OutArray;
	}

	// Gets the current Dialogue Participant Names.
	TArray<FName> GetCurrentDialogueParticipantNames() const
	{
		return FDialogueDetailsPanelUtils::GetDialogueSortedParticipantNames(Dialogue);
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
	// The current Event type of the struct.
	EDlgEventType EventType = EDlgEventType::Event;

	// Cache the some property handles
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle;

	// Cache the properties
	TSharedPtr<IPropertyHandle> EventTypePropertyHandle;

	// just some nice utilities
	TSharedPtr<IPropertyUtilities> PropertyUtils;

	// Cache the rows of the properties, created in CustomizeChildren
	TSharedPtr<FDialogueTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FDialogueTextPropertyPickList_CustomRowHelper> EventNamePropertyRow;
	IDetailPropertyRow* IntValuePropertyRow = nullptr;
	IDetailPropertyRow* FloatValuePropertyRow = nullptr;
	IDetailPropertyRow* NameValuePropertyRow = nullptr;
	IDetailPropertyRow* BoolDeltaPropertyRow = nullptr;
	IDetailPropertyRow* BoolValuePropertyRow = nullptr;

	IDetailPropertyRow* CustomEventPropertyRow = nullptr;
	TSharedPtr<FDialogueObject_CustomRowHelper> CustomEventPropertyRow_CustomDisplay;

	IDetailPropertyRow* EventTypePropertyRow = nullptr;
	TSharedPtr<FDialogueEnumTypeWithObject_CustomRowHelper> EventTypePropertyRow_CustomDisplay;

	// Hold a reference to dialogue we are displaying.
	UDlgDialogue* Dialogue = nullptr;
};
