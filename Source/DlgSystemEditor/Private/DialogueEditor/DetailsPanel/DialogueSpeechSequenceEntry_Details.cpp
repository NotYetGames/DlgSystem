// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueSpeechSequenceEntry_Details.h"

#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"

#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "Widgets/SDialogueTextPropertyPickList.h"
#include "Widgets/DialogueTextPropertyPickList_CustomRowHelper.h"
#include "Widgets/DialogueMultiLineEditableTextBox_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueSpeechSequenceEntry_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSpeechSequenceEntry_Details
void FDialogueSpeechSequenceEntry_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDialogueDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDialogueSpeechSequenceEntry_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const bool bHasDialogue = Dialogue != nullptr;

	// Speaker
	{
		const TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, Speaker));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		.Update();
	}

	// Text
	{
		TextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, Text));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

		TextPropertyRow = MakeShared<FDialogueMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, TextPropertyHandle);
		TextPropertyRow->SetPropertyUtils(StructCustomizationUtils.GetPropertyUtilities());
		TextPropertyRow->Update();
	}


	// Edge Text
	{
		EdgeTextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, EdgeText));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("EdgeTextSearchKey", "Edge Text"));

		EdgeTextPropertyRow = MakeShared<FDialogueMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, EdgeTextPropertyHandle);
		EdgeTextPropertyRow->SetPropertyUtils(StructCustomizationUtils.GetPropertyUtilities());
		EdgeTextPropertyRow->Update();
	}

	//
	// Data
	//

	// Speaker State
	{
		const TSharedPtr<IPropertyHandle> SpeakerStatePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, SpeakerState));

		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("SpeakerStateSearchKey", "Speaker State"));

		SpeakerStatePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, SpeakerStatePropertyHandle);
		SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesSpeakerStates)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(false)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetSpeakerStateNodeVisibility))
		.Update();
	}

	// Node Data that can be anything set by the user
	NodeDataPropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, NodeData)).ToSharedRef());
	NodeDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetNodeDataVisibility));

	// SoundWave
	VoiceSoundWavePropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, VoiceSoundWave)).ToSharedRef());
	VoiceSoundWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetVoiceSoundWaveVisibility));

	// DialogueWave
	VoiceDialogueWavePropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, VoiceDialogueWave)).ToSharedRef());
	VoiceDialogueWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetVoiceDialogueWaveVisibility));

	// Generic Data, can be FMOD sound
	GenericDataPropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, GenericData)).ToSharedRef());
	GenericDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetNodeGenericDataVisibility));
}

#undef LOCTEXT_NAMESPACE
