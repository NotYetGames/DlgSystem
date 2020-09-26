// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueGraphNode_Details.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#include "Nodes/DlgNode.h"
#include "Nodes/DlgNode_SpeechSequence.h"
#include "Nodes/DlgNode_Speech.h"
#include "Nodes/DlgNode_Selector.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "Widgets/SDialogueTextPropertyPickList.h"
#include "Widgets/DialogueTextPropertyPickList_CustomRowHelper.h"
#include "Widgets/DialogueMultiLineEditableTextBox_CustomRowHelper.h"
#include "Widgets/DialogueObject_CustomRowHelper.h"
#include "Widgets/Input/SButton.h"

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

	GraphNode = WeakGraphNode.Get();
	if (!IsValid(GraphNode))
	{
		return;
	}

	DetailLayoutBuilder = &DetailBuilder;
	Dialogue = GraphNode->GetDialogue();
	const UDlgNode& DialogueNode = GraphNode->GetDialogueNode();
	const bool bIsRootNode = GraphNode->IsRootNode();
	const bool bIsEndNode = GraphNode->IsEndNode();
	const bool bIsSpeechNode = GraphNode->IsSpeechNode();
	const bool bIsSelectorNode = GraphNode->IsSelectorNode();;
	const bool bIsSpeechSequenceNode = GraphNode->IsSpeechSequenceNode();
	const bool bIsVirtualParentNode = GraphNode->IsVirtualParentNode();

	// Hide the existing category
	DetailLayoutBuilder->HideCategory(UDialogueGraphNode::StaticClass()->GetFName());

	// Fill with the properties of the DialogueNode
	IDetailCategoryBuilder& BaseDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Base Node"), FText::GetEmpty(), ECategoryPriority::Important);
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

			ParticipantNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
			ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
				SNew(SDialogueTextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetAllDialoguesParticipantNames)
				.OnTextCommitted(this, &Self::HandleParticipantTextCommitted)
				.HasContextCheckbox(true)
				.IsContextCheckBoxChecked(true)
				.CurrentContextAvailableSuggestions(this, &Self::GetCurrentDialogueParticipantNames)
			)
			.Update();
		}

		// End Nodes can't have children
		if (!bIsEndNode)
		{
			BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameCheckChildrenOnEvaluation()));
		}

		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameEnterConditions()))
			.ShouldAutoExpand(true);
		BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameEnterEvents()))
			.ShouldAutoExpand(true);
	}

	// GUID
	BaseDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameGUID()))
		.ShouldAutoExpand(true);

	if (!bIsEndNode)
	{
		ChildrenPropertyRow = &BaseDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode::GetMemberNameChildren())
		);
		ChildrenPropertyRow->ShouldAutoExpand(true);
		ChildrenPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetChildrenVisibility));
	}


	// Do nothing
	if (bIsRootNode)
	{
		return;
	}

	// NOTE: be careful here with the child handle names that are common. For example 'Text' so that we do not get the child
	// property from some inner properties
	if (bIsSpeechNode)
	{
		IDetailCategoryBuilder& SpeechCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Node"));
		SpeechCategory.InitiallyCollapsed(false);

		// bIsVirtualParent
		IsVirtualParentPropertyHandle = PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameIsVirtualParent());
		IsVirtualParentPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &Self::OnIsVirtualParentChanged));
		SpeechCategory.AddProperty(IsVirtualParentPropertyHandle);

		// bVirtualParentFireDirectChildEnterEvents
		if (bIsVirtualParentNode)
		{
			SpeechCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVirtualParentFireDirectChildEnterEvents()))
				.ShouldAutoExpand(true);
		}

		// Text
		{
			TextPropertyHandle = PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameText());
			FDetailWidgetRow* DetailWidgetRow = &SpeechCategory.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

			TextPropertyRow = MakeShared<FDialogueMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, TextPropertyHandle);
			TextPropertyRow->SetPropertyUtils(DetailBuilder.GetPropertyUtilities());
			TextPropertyRow->Update();

			TextPropertyRow->OnTextCommittedEvent().AddRaw(this, &Self::HandleTextCommitted);
			TextPropertyRow->OnTextChangedEvent().AddRaw(this, &Self::HandleTextChanged);
		}

		// Text arguments
		SpeechCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameTextArguments()))
			.ShouldAutoExpand(true);

		//
		// Data
		//

		IDetailCategoryBuilder& SpeechDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Node Data"));
		SpeechDataCategory.InitiallyCollapsed(false);

		// Speaker State
		{
			const TSharedPtr<IPropertyHandle> SpeakerStatePropertyHandle =
				PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameSpeakerState());

			FDetailWidgetRow* DetailWidgetRow = &SpeechDataCategory.AddCustomRow(LOCTEXT("SpeakerStateSearchKey", "Speaker State"));

			SpeakerStatePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, SpeakerStatePropertyHandle);
			SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
				SNew(SDialogueTextPropertyPickList)
				.AvailableSuggestions(this, &Self::GetAllDialoguesSpeakerStates)
				.OnTextCommitted(this, &Self::HandleSpeakerStateCommitted)
				.HasContextCheckbox(false)
			)
			.SetVisibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetSpeakerStateNodeVisibility))
			.Update();
		}

		// Node Data that can be anything set by the user
		NodeDataPropertyRow = &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameNodeData())
		);
		NodeDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetNodeDataVisibility));
		NodeDataPropertyRow->ShouldAutoExpand(true);
		NodeDataPropertyRow->ShowPropertyButtons(true);

		// Add Custom buttons
		NodeDataPropertyRow_CustomDisplay = MakeShared<FDialogueObject_CustomRowHelper>(NodeDataPropertyRow);
		NodeDataPropertyRow_CustomDisplay->Update();

		// SoundWave
		VoiceSoundWavePropertyRow = &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVoiceSoundWave())
		);
		VoiceSoundWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetVoiceSoundWaveVisibility));

		// DialogueWave
		VoiceDialogueWavePropertyRow =  &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameVoiceDialogueWave())
		);
		VoiceDialogueWavePropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetVoiceDialogueWaveVisibility));

		// Generic Data, can be FMOD sound
		GenericDataPropertyRow = &SpeechDataCategory.AddProperty(
			PropertyDialogueNode->GetChildHandle(UDlgNode_Speech::GetMemberNameGenericData())
		);
		GenericDataPropertyRow->Visibility(CREATE_VISIBILITY_CALLBACK_STATIC(&FDialogueDetailsPanelUtils::GetNodeGenericDataVisibility));
	}
	else if (bIsSelectorNode)
	{
		IDetailCategoryBuilder& SpeechDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Selector Node"));
		SpeechDataCategory.InitiallyCollapsed(false);
		SpeechDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_Selector::GetMemberNameSelectorType()));
	}
	else if (bIsSpeechSequenceNode)
	{
		IDetailCategoryBuilder& SpeechSequenceDataCategory = DetailLayoutBuilder->EditCategory(TEXT("Speech Sequence Node"));
		SpeechSequenceDataCategory.InitiallyCollapsed(false)
			.RestoreExpansionState(true);
		SpeechSequenceDataCategory.AddProperty(PropertyDialogueNode->GetChildHandle(UDlgNode_SpeechSequence::GetMemberNameSpeechSequence()))
			.ShouldAutoExpand(true);;
	}
}

void FDialogueGraphNode_Details::HandleTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	// Text arguments already handled in node post change properties
}

void FDialogueGraphNode_Details::HandleTextChanged(const FText& InText)
{
	if (GraphNode)
	{
		GraphNode->GetMutableDialogueNode()->RebuildTextArgumentsFromPreview(InText);
	}
}


void FDialogueGraphNode_Details::OnIsVirtualParentChanged()
{
	DetailLayoutBuilder->ForceRefreshDetails();
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
