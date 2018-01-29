// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueSpeechSequenceEntry_Details.h"

#include "IPropertyUtilities.h"
#include "PropertyEditing.h"

#include "DlgNode.h"
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
	Dialogue = DetailsPanel::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());

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
	// Speaker
	{
		const TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, Speaker));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("ParticipantNameSearchKey", "Participant Name"));

		ParticipantNamePropertyRow = MakeShareable(new FTextPropertyPickList_CustomRowHelper(DetailWidgetRow, ParticipantNamePropertyHandle));
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.HasContextCheckbox(true)
			.IsContextCheckBoxChecked(true)
			.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
		)
		->Update();
	}

	// Text
	{
		TextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgSpeechSequenceEntry, Text));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

		TextPropertyRow = MakeShareable(new FMultiLineEditableTextBox_CustomRowHelper(DetailWidgetRow, TextPropertyHandle));
		TextPropertyRow->SetMultiLineEditableTextBoxWidget(
			SNew(SMultiLineEditableTextBox)
			.Font(FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
			.SelectAllTextWhenFocused(false)
			.ClearKeyboardFocusOnCommit(false)
			.SelectAllTextOnCommit(false)
			.AutoWrapText(true)
			.ModiferKeyForNewLine(DetailsPanel::GetModifierKeyFromDialogueSettings())
			.Text(TextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::GetTextValue)
			.OnTextCommitted(TextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::HandleTextCommited)
		)
		->Update();
	}

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

		EdgeTextPropertyRow = MakeShareable(new FMultiLineEditableTextBox_CustomRowHelper(DetailWidgetRow, EdgeTextPropertyHandle));
		EdgeTextPropertyRow->SetMultiLineEditableTextBoxWidget(
			SNew(SMultiLineEditableTextBox)
			.Font(FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
			.SelectAllTextWhenFocused(false)
			.ClearKeyboardFocusOnCommit(false)
			.SelectAllTextOnCommit(false)
			.AutoWrapText(true)
			.ModiferKeyForNewLine(DetailsPanel::GetModifierKeyFromDialogueSettings())
			.Text(EdgeTextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::GetTextValue)
			.OnTextCommitted(EdgeTextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::HandleTextCommited)
		)
		->Update();
	}
}

#undef LOCTEXT_NAMESPACE
