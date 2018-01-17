// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Visibility.h"
#include "IDetailPropertyRow.h"

#include "DlgDialogue.h"
#include "DlgEvent.h"
#include "DlgManager.h"
#include "DialogueDetailsPanelUtils.h"

class FTextPropertyPickList_CustomRowHelper;

/**
 * How the details panel renders the FDlgEvent
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class FDialogueEvent_Details : public IPropertyTypeCustomization
{
	typedef FDialogueEvent_Details Self;

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
	// Called every time the event type is changed
	void OnEventTypeChanged(bool bForceRefresh);

	// Getters for the visibility of some properties
	EVisibility GetIntValueVisibility() const
	{
		return EventType == EDlgEventType::DlgEventModifyInt ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetFloatValueVisibility() const
	{
		return EventType == EDlgEventType::DlgEventModifyFloat ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetNameValueVisibility() const
	{
		return EventType == EDlgEventType::DlgEventModifyName ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetBoolDeltaVisibility() const
	{
		return EventType == EDlgEventType::DlgEventModifyInt || EventType == EDlgEventType::DlgEventModifyFloat
			? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetBoolValueVisibility() const
	{
		return EventType == EDlgEventType::DlgEventModifyBool ? EVisibility::Visible : EVisibility::Hidden;
	}

	/** Gets all the event name suggestions depending on EventType from all Dialogues. */
	TArray<FName> GetAllDialoguesEventNames() const
	{
		TArray<FName> Suggestions;
		const FName ParticipantName = DetailsPanel::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());

		switch (EventType)
		{
		case EDlgEventType::DlgEventModifyBool:
			UDlgManager::GetAllDialoguesBoolNames(ParticipantName, Suggestions);
			break;

		case EDlgEventType::DlgEventModifyFloat:
			UDlgManager::GetAllDialoguesFloatNames(ParticipantName, Suggestions);
			break;

		case EDlgEventType::DlgEventModifyInt:
			UDlgManager::GetAllDialoguesIntNames(ParticipantName, Suggestions);
			break;

		case EDlgEventType::DlgEventModifyName:
			UDlgManager::GetAllDialoguesNameNames(ParticipantName, Suggestions);
			break;

		case EDlgEventType::DlgEventEvent:
		default:
			UDlgManager::GetAllDialoguesEventNames(ParticipantName, Suggestions);
			break;
		}

		return Suggestions;
	}

	/** Gets all the event name suggestions depending on EventType from the current Dialogue */
	TArray<FName> GetCurrentDialogueEventNames() const
	{
		const FName ParticipantName = DetailsPanel::GetParticipantNameFromPropertyHandle(ParticipantNamePropertyHandle.ToSharedRef());
		TSet<FName> Names;

		switch (EventType)
		{
		case EDlgEventType::DlgEventModifyBool:
			Dialogue->GetBoolNames(ParticipantName, Names);
			break;

		case EDlgEventType::DlgEventModifyName:
			Dialogue->GetNameNames(ParticipantName, Names);
			break;

		case EDlgEventType::DlgEventModifyFloat:
			Dialogue->GetFloatNames(ParticipantName, Names);
			break;

		case EDlgEventType::DlgEventModifyInt:
			Dialogue->GetIntNames(ParticipantName, Names);
			break;

		case EDlgEventType::DlgEventEvent:
		default:
			Dialogue->GetEvents(ParticipantName, Names);
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

	/** Gets the current Dialogue Participant Names. */
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
	// The current Event type of the struct.
	EDlgEventType EventType = EDlgEventType::DlgEventEvent;

	// Cache the some property handles
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle;

	// Cache the properties
	TSharedPtr<IPropertyHandle> EventTypePropertyHandle;

	// just some nice utilities
	TSharedPtr<IPropertyUtilities> PropertyUtils;

	// Cache the rows of the properties, created in CustomizeChildren
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> EventNamePropertyRow;
	IDetailPropertyRow* IntValuePropertyRow = nullptr;
	IDetailPropertyRow* FloatValuePropertyRow = nullptr;
	IDetailPropertyRow* NameValuePropertyRow = nullptr;
	IDetailPropertyRow* BoolDeltaPropertyRow = nullptr;
	IDetailPropertyRow* BoolValuePropertyRow = nullptr;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;
};
