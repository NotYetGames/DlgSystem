// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgSpeechSequenceEntry_Details.h"

#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"

#include "DlgDetailsPanelUtils.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/SDlgTextPropertyPickList.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgTextPropertyPickList_CustomRowHelper.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgMultiLineEditableTextBox_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueSpeechSequenceEntry_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgSpeechSequenceEntry_Details
void FDlgSpeechSequenceEntry_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDlgDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDlgSpeechSequenceEntry_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const bool bHasDialogue = Dialogue != nullptr;

	// Speaker
	{
		const TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, Speaker));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShared<FDlgTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDlgTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetDialoguesParticipantNames)
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

		TextPropertyRow = MakeShared<FDlgMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, TextPropertyHandle);
		TextPropertyRow->SetPropertyUtils(StructCustomizationUtils.GetPropertyUtilities());
		TextPropertyRow->Update();
	}


	// Edge Text
	{
		EdgeTextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, EdgeText));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("EdgeTextSearchKey", "Edge Text"));

		EdgeTextPropertyRow = MakeShared<FDlgMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, EdgeTextPropertyHandle);
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

		SpeakerStatePropertyRow = MakeShared<FDlgTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, SpeakerStatePropertyHandle);
		SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDlgTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetDialoguesSpeakerStates)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(false)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetSpeakerStateNodeVisibility))
		.Update();
	}

	// Node Data that can be anything set by the user
	NodeDataPropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, NodeData)).ToSharedRef());
	NodeDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetNodeDataVisibility));

	// SoundWave
	VoiceSoundWavePropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, VoiceSoundWave)).ToSharedRef());
	VoiceSoundWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetVoiceSoundWaveVisibility));

	// DialogueWave
	VoiceDialogueWavePropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, VoiceDialogueWave)).ToSharedRef());
	VoiceDialogueWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetVoiceDialogueWaveVisibility));

	// Generic Data, can be FMOD sound
	GenericDataPropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, GenericData)).ToSharedRef());
	GenericDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDlgDetailsPanelUtils::GetNodeGenericDataVisibility));
}

#undef LOCTEXT_NAMESPACE
