// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "PropertyEditorModule.h"
#include "IDetailCustomization.h"
#include "IDetailPropertyRow.h"

#include "DlgManager.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueDetailsPanelUtils.h"

class FDialogueObject_CustomRowHelper;
class FDialogueTextPropertyPickList_CustomRowHelper;
class FDialogueMultiLineEditableTextBox_CustomRowHelper;

/**
 * How the details customization panel looks for UDialogueGraphNode object
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class FDialogueGraphNode_Details : public IDetailCustomization
{
	typedef FDialogueGraphNode_Details Self;

public:
	// Makes a new instance of this detail layout class for a specific detail view requesting it
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<Self>(); }

	// IDetailCustomization interface
	/** Called when details should be customized */
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	/** Handler for when the text is changed */
	void HandleTextCommitted(const FText& InText, ETextCommit::Type CommitInfo);
	void HandleTextChanged(const FText& InText);


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

	/** Gets the Speaker States from all Dialogues. */
	TArray<FName> GetAllDialoguesSpeakerStates() const
	{
		TArray<FName> OutArray;
		UDlgManager::GetAllDialoguesSpeakerStates(OutArray);
		return OutArray;
	}

	/** Handler for when text in the editable text box changed */
	void HandleParticipantTextCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo)
	{
		Dialogue->UpdateAndRefreshData();
	}

	/** Handler for when the speaker state is changed */
	void HandleSpeakerStateCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo)
	{
		Dialogue->UpdateAndRefreshData();
	}

	// The IsVirtualParent property changed
	void OnIsVirtualParentChanged();

private:
	/** Hold the reference to the Graph Node this represents */
	UDialogueGraphNode* GraphNode = nullptr;

	/** Cache some properties. */
	// Property Handles
	TSharedPtr<IPropertyHandle> IsVirtualParentPropertyHandle;
	TSharedPtr<IPropertyHandle> TextPropertyHandle;

	// Property rows
	TSharedPtr<FDialogueTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FDialogueTextPropertyPickList_CustomRowHelper> SpeakerStatePropertyRow;
	TSharedPtr<FDialogueMultiLineEditableTextBox_CustomRowHelper> TextPropertyRow;
	IDetailPropertyRow* NodeDataPropertyRow = nullptr;
	TSharedPtr<FDialogueObject_CustomRowHelper> NodeDataPropertyRow_CustomDisplay;
	IDetailPropertyRow* VoiceSoundWavePropertyRow = nullptr;
	IDetailPropertyRow* VoiceDialogueWavePropertyRow = nullptr;
	IDetailPropertyRow* GenericDataPropertyRow = nullptr;
	IDetailPropertyRow* ChildrenPropertyRow = nullptr;

	/** The details panel layout builder reference. */
	IDetailLayoutBuilder* DetailLayoutBuilder = nullptr;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;
};
