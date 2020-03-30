// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"
#include "IDetailPropertyRow.h"

#include "DlgManager.h"
#include "DialogueDetailsPanelUtils.h"

class FDialogueTextPropertyPickList_CustomRowHelper;
class FDialogueMultiLineEditableTextBox_CustomRowHelper;

/**
* How the details customization panel looks for the FDlgSpeechSequenceEntry
* See FDlgSystemEditorModule::StartupModule for usage.
*/
class FDialogueSpeechSequenceEntry_Details : public IPropertyTypeCustomization
{
	typedef FDialogueSpeechSequenceEntry_Details Self;

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
	/** Gets the ParticipantNames from all Dialogues. */
	TArray<FName> GetAllDialoguesParticipantNames() const
	{
		TArray<FName> OutArray;
		UDlgManager::GetAllDialoguesParticipantNames(OutArray);
		return OutArray;
	}

	/** Gets the Speaker States from all Dialogues. */
	TArray<FName> GetAllDialoguesSpeakerStates() const
	{
		TArray<FName> OutArray;
		UDlgManager::GetAllDialoguesSpeakerStates(OutArray);
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
			Dialogue->UpdateAndRefreshData();
		}
	}

private:
	/** The property handle of the entire struct. */
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	// Cache the rows of the properties, created in CustomizeChildren
	TSharedPtr<IPropertyHandle> TextPropertyHandle;
	TSharedPtr<IPropertyHandle> EdgeTextPropertyHandle;

	TSharedPtr<FDialogueTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FDialogueTextPropertyPickList_CustomRowHelper> SpeakerStatePropertyRow;
	IDetailPropertyRow* VoiceSoundWavePropertyRow = nullptr;
	IDetailPropertyRow* VoiceDialogueWavePropertyRow = nullptr;
	IDetailPropertyRow* GenericDataPropertyRow = nullptr;
	IDetailPropertyRow* NodeDataPropertyRow = nullptr;
	TSharedPtr<FDialogueMultiLineEditableTextBox_CustomRowHelper> TextPropertyRow;
	TSharedPtr<FDialogueMultiLineEditableTextBox_CustomRowHelper> EdgeTextPropertyRow;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;
};
