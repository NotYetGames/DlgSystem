// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "PropertyEditorModule.h"
#include "IDetailCustomization.h"
#include "IDetailPropertyRow.h"

#include "DlgManager.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueDetailsPanelUtils.h"

class FTextPropertyPickList_CustomRowHelper;
class FMultiLineEditableTextBox_CustomRowHelper;

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
		Dialogue->RefreshData();
	}

	/** Handler for when the speaker state is changed */
	void HandleSpeakerStateCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo)
	{
		Dialogue->RefreshData();
	}

	// The IsVirtualParent property changed
	void OnIsVirtualParentChanged();

	// Getters for visibility of some properties
	EVisibility GetVoiceSoundWaveVisibility() const
	{
		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		return Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::DlgVoiceDisplayedSoundWave ||
			   Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::DlgVoiceDisplayedSoundWaveAndDialogueWave
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetVoiceDialogueWaveVisibility() const
	{
		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		return Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::DlgVoiceDisplayedDialogueWave ||
			   Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::DlgVoiceDisplayedSoundWaveAndDialogueWave
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetGenericDataVisibility() const
	{
		return GetDefault<UDlgSystemSettings>()->bShowGenericData ? EVisibility::Visible : EVisibility::Hidden;
	}

	EVisibility GetSpeakerStateVisibility() const
	{
		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		return Settings->DialogueSpeakerStateVisibility == EDlgSpeakerStateVisibility::DlgShowOnNode ||
			   Settings->DialogueSpeakerStateVisibility == EDlgSpeakerStateVisibility::DlgShowOnNodeAndEdge
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

private:
	/** Hold the reference to the Graph Node this represents */
	UDialogueGraphNode* GraphNode = nullptr;

	/** Cache some properties. */
	// Property Handles
	TSharedPtr<IPropertyHandle> IsVirtualParentPropertyHandle;
	TSharedPtr<IPropertyHandle> TextPropertyHandle;

	// Property rows
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
	TSharedPtr<FTextPropertyPickList_CustomRowHelper> SpeakerStatePropertyRow;
	TSharedPtr<FMultiLineEditableTextBox_CustomRowHelper> TextPropertyRow;
	IDetailPropertyRow* VoiceSoundWavePropertyRow = nullptr;
	IDetailPropertyRow* VoiceDialogueWavePropertyRow = nullptr;
	IDetailPropertyRow* GenericDataPropertyRow = nullptr;

	/** The details panel layout builder reference. */
	IDetailLayoutBuilder* DetailLayoutBuilder = nullptr;

	/** Hold a reference to dialogue we are displaying. */
	UDlgDialogue* Dialogue = nullptr;
};
