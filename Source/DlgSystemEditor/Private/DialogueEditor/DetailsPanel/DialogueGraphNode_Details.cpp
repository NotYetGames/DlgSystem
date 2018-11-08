// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueGraphNode_Details.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#include "Nodes/DlgNode.h"
#include "Nodes/DlgNode_SpeechSequence.h"
#include "Nodes/DlgNode_Speech.h"
#include "Nodes/DlgNode_Selector.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "STextPropertyPickList.h"
#include "CustomRowHelpers/TextPropertyPickList_CustomRowHelper.h"
#include "CustomRowHelpers/MultiLineEditableTextBox_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialoguGraphNode_Details"

void FDialogueGraphNode_Details::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	// Only support one object being customized
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	TWeakObjectPtr<UDialogueGraphNode> WeakGraphNode = Cast<UDialogueGraphNode>(ObjectsBeingCustomized[0].Get());
	if (!WeakGraphNode.IsValid())
	{
		return;
	}

	UDialogueGraphNode* TempGraphNode = WeakGraphNode.Get();
	if (!IsValid(TempGraphNode))
	{
		return;
	}

	DetailLayoutBuilder = &DetailBuilder;
	GraphNode = TempGraphNode;
	Dialogue = GraphNode->GetDialogue();
	const UDlgNode& DialogueNode = GraphNode->GetDialogueNode();
	const bool bIsRootNode = GraphNode->IsRootNode();
	const bool bIsEndNode = GraphNode->IsEndNode();
	const bool bIsSpeechNode = GraphNode->IsSpeechNode();
	const bool bIsSelectorNode = GraphNode->IsSelectorNode();;
	const bool bIsSpeechSequenceNode = GraphNode->IsSpeechSequenceNode();

	// Hide the existing category
	DetailLayoutBuilder->HideCategory(UDialogueGraphNode::StaticClass()->GetFName());

	// Fill with the properties of the DialogueNode
	IDetailCategoryBuilder& BaseDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Base Node Data"));
	BaseDataCategory.InitiallyCollapsed(false);
	const TSharedPtr<IPropertyHandle> PropertyDialogueNode =
		DetailLayoutBuilder->GetProperty(UDialogueGraphNode::GetMemberNameDialogueNode(), UDialogueGraphNode::StaticClass());

	if (!bIsRootNode)
	{
		// NodeIndex
		BaseDataCategory.AddProperty(UDialogueGraphNode::GetMemberNameNodeIndex(), UDialogueGraphNode::StaticClass());

		// OwnerName
		{
			const TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle =
				PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameOwnerName());
			FDetailWidgetRow* DetailWidgetRow = &BaseDataCategory.AddCustomRow(LOCTEXT("ParticipantNameSearcKey", "Participant Name"));

			ParticipantNamePropertyRow = MakeShareable(new FTextPropertyPickList_CustomRowHelper(DetailWidgetRow, ParticipantNamePropertyHandle));
			ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
				SNew(STextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
				.OnTextCommitted(this, &Self::HandleParticipantTextCommitted)
				.HasContextCheckbox(true)
				.IsContextCheckBoxChecked(true)
				.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
			)
			->Update();
		}

		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameCheckChildrenOnEvaluation()));
		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameEnterConditions()))
			.ShouldAutoExpand(true);
		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameEnterEvents()))
			.ShouldAutoExpand(true);
	}
	if (!bIsEndNode)
	{
		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameChildren()))
			.ShouldAutoExpand(true);
	}

	// NOTE: be careful here with the child handle names that are common. For example 'Text' so that we do not get the child
	// property from some inner properties
	if (bIsSpeechNode)
	{
		IDetailCategoryBuilder& SpeechDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Node Data"));
		SpeechDataCategory.InitiallyCollapsed(false);

		if (!bIsRootNode)
		{
			IsVirtualParentPropertyHandle = PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameIsVirtualParent());
			IsVirtualParentPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &Self::OnIsVirtualParentChanged));
			SpeechDataCategory.AddProperty(IsVirtualParentPropertyHandle);

			// Speaker State
			{
				const TSharedPtr<IPropertyHandle> SpeakerStatePropertyHandle =
					PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameSpeakerState());

				FDetailWidgetRow* DetailWidgetRow = &SpeechDataCategory.AddCustomRow(LOCTEXT("SpeakerStateSearchKey", "Speaker State"));

				SpeakerStatePropertyRow = MakeShareable(new FTextPropertyPickList_CustomRowHelper(DetailWidgetRow, SpeakerStatePropertyHandle));
				SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
					SNew(STextPropertyPickList)
					.AvailableSuggestions(this, &Self::GetAllDialoguesSpeakerStates)
					.OnTextCommitted(this, &Self::HandleSpeakerStateCommitted)
					.HasContextCheckbox(false)
				)
				->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetSpeakerStateVisibility))
				->Update();
			}

			// Text
			{
				TextPropertyHandle = PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameText());
				FDetailWidgetRow* DetailWidgetRow = &SpeechDataCategory.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

				TextPropertyRow = MakeShareable(new FMultiLineEditableTextBox_CustomRowHelper(DetailWidgetRow, TextPropertyHandle));
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
				->SetPropertyUtils(DetailBuilder.GetPropertyUtilities())
				->Update();
			}

			// text arguments
			SpeechDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameTextArguments()))
				.ShouldAutoExpand(true);

			// Voice

			// SoundWave
			VoiceSoundWavePropertyRow = &SpeechDataCategory.AddProperty(
				PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVoiceSoundWave()));
			VoiceSoundWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetVoiceSoundWaveVisibility));

			// DialogueWave
			VoiceDialogueWavePropertyRow =  &SpeechDataCategory.AddProperty(
				PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVoiceDialogueWave()));
			VoiceDialogueWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetVoiceDialogueWaveVisibility));
		}
	}
	else if (bIsSelectorNode)
	{
		IDetailCategoryBuilder& SpeechDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Selector Node Data"));
		SpeechDataCategory.InitiallyCollapsed(false);
		SpeechDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Selector::GetMemberNameSelectorType()));
	}
	else if (bIsSpeechSequenceNode)
	{
		IDetailCategoryBuilder& SpeechSequenceDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Sequence Node Data"));
		SpeechSequenceDataCategory.InitiallyCollapsed(false);
		SpeechSequenceDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_SpeechSequence::GetMemberNameSpeechSequence()))
			.ShouldAutoExpand(true);;
	}
}

void FDialogueGraphNode_Details::OnIsVirtualParentChanged()
{
	DetailLayoutBuilder->ForceRefreshDetails();
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
