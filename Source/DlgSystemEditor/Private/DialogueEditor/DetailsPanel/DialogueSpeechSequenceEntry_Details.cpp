// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueSpeechSequenceEntry_Details.h"

#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"

#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "STextPropertyPickList.h"
#include "CustomRowHelpers/TextPropertyPickList_CustomRowHelper.h"
#include "CustomRowHelpers/MultiLineEditableTextBox_CustomRowHelper.h"

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

		ParticipantNamePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(bHasDialogue)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		->Update();
	}

	// Speaker State
	{
		const TSharedPtr<IPropertyHandle> SpeakerStatePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, SpeakerState));

		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("SpeakerStateSearchKey", "Speaker State"));

		SpeakerStatePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, SpeakerStatePropertyHandle);
		SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesSpeakerStates)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(false)
		)
		->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetSpeakerStateVisibility))
		->Update();
	}

	// Text
	{
		TextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, Text));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

		TextPropertyRow = MakeShared<FMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, TextPropertyHandle);
		TextPropertyRow->SetMultiLineEditableTextBoxWidget(
			SNew(SMultiLineEditableTextBox)
			.Font(FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
			.SelectAllTextWhenFocused(false)
			.ClearKeyboardFocusOnCommit(false)
			.SelectAllTextOnCommit(false)
			.AutoWrapText(true)
			.ModiferKeyForNewLine(FDialogueDetailsPanelUtils::GetModifierKeyFromDialogueSettings())
			.Text(TextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::GetTextValue)
			.OnTextCommitted(TextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::HandleTextCommited)
		)
		->SetPropertyUtils(StructCustomizationUtils.GetPropertyUtilities())
		->Update();
	}

	// Node Data
	NodeDataPropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, NodeData)).ToSharedRef());

	// Voice
	VoiceSoundWavePropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, VoiceSoundWave)).ToSharedRef());
	VoiceSoundWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetVoiceSoundWaveVisibility));

	VoiceDialogueWavePropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, VoiceDialogueWave)).ToSharedRef());
	VoiceDialogueWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetVoiceDialogueWaveVisibility));

	// Edge Text
	{
		EdgeTextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, EdgeText));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("EdgeTextSearchKey", "Edge Text"));

		EdgeTextPropertyRow = MakeShared<FMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, EdgeTextPropertyHandle);
		EdgeTextPropertyRow->SetMultiLineEditableTextBoxWidget(
			SNew(SMultiLineEditableTextBox)
			.Font(FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
			.SelectAllTextWhenFocused(false)
			.ClearKeyboardFocusOnCommit(false)
			.SelectAllTextOnCommit(false)
			.AutoWrapText(true)
			.ModiferKeyForNewLine(FDialogueDetailsPanelUtils::GetModifierKeyFromDialogueSettings())
			.Text(EdgeTextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::GetTextValue)
			.OnTextCommitted(EdgeTextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::HandleTextCommited)
		)
		->SetPropertyUtils(StructCustomizationUtils.GetPropertyUtilities())
		->Update();
	}

	// Generic Data
	GenericDataPropertyRow = &StructBuilder.AddProperty(
		StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, GenericData)).ToSharedRef());
	GenericDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetGenericDataVisibility));
}

#undef LOCTEXT_NAMESPACE
