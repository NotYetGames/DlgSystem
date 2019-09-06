// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"

#include "DlgTextArgument.h"
#include "DialogueDetailsPanelUtils.h"

class FTextPropertyPickList_CustomRowHelper;

/**
 * How the details panel renders the FDlgTextArgument
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class FDialogueTextArgument_Details : public IPropertyTypeCustomization
{
	typedef FDialogueTextArgument_Details Self;

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
	void OnArgumentTypeChanged(bool bForceRefresh);

	// Getters for the visibility of some properties
	EVisibility GetVariableNameVisibility() const
	{
		return (ArgumentType != EDlgTextArgumentType::DlgTextArgumentDisplayName && ArgumentType != EDlgTextArgumentType::DlgTextArgumentGender)
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	/** Gets all the event name suggestions depending on EventType from all Dialogues. */
	TArray<FName> GetAllDialoguesVariableNames() const
	{
		return GetDialogueVariableNames(false);
	}

	/** Gets all the event name suggestions depending on EventType from the current Dialogue */
	TArray<FName> GetCurrentDialogueVariableNames() const
	{
		return GetDialogueVariableNames(true);
	}

	TArray<FName> GetDialogueVariableNames(bool bCurrentOnly) const;


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
	// The current Event type of the struct.
	EDlgTextArgumentType ArgumentType = EDlgTextArgumentType::DlgTextArgumentDisplayName;

	// Cache the some property handles
	TSharedPtr<IPropertyHandle> StructPropertyHandle;
	TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle;

	// Cache the properties
	TSharedPtr<IPropertyHandle> ArgumentTypePropertyHandle;

	// just some nice utilities
	TSharedPtr<IPropertyUtilities> PropertyUtils;

	// Cache the rows of the properties, created in CustomizeChildren
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> VariableNamePropertyRow;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;
};
