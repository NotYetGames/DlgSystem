// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueEdge_Details.h"

#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyTypeCustomization.h"

#include "Nodes/DlgNode.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "Widgets/DialogueMultiLineEditableTextBox_CustomRowHelper.h"
#include "Widgets/SDialogueTextPropertyPickList.h"
#include "Widgets/DialogueTextPropertyPickList_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueEdge_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueEdge_Details
void FDialogueEdge_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDialogueDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	bShowTextProperty = true;

	// Should we show hide the Text property?
	if (const UDialogueGraphNode* GraphNode = FDialogueDetailsPanelUtils::GetClosestGraphNodeFromPropertyHandle(StructPropertyHandle.ToSharedRef()))
	{
		// Virtual parents do not handle direct children, only grand children
		// And selector node do not even touch them
		bShowTextProperty = FDlgEdge::IsTextVisible(GraphNode->GetDialogueNode());

		// Special case
		// Selector node but one of its parent is a virtual parent, allow text then
		if (!bShowTextProperty && GraphNode->IsSelectorNode())
		{
			for (const UDialogueGraphNode* ParentNode : GraphNode->GetParentNodes())
			{
				if (ParentNode->IsVirtualParentNode())
				{
					bShowTextProperty = true;
					break;
				}
			}
		}
	}

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDialogueEdge_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// TargetIndex
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, TargetIndex)).ToSharedRef());

	// Conditions
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, Conditions)).ToSharedRef());

	// Text
	TextPropertyHandle = StructPropertyHandle->GetChildHandle(FDlgEdge::GetMemberNameText());
	FDetailWidgetRow* TextDetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

	TextPropertyRow = MakeShared<FDialogueMultiLineEditableTextBox_CustomRowHelper>(TextDetailWidgetRow, TextPropertyHandle);
	TextPropertyRow->SetPropertyUtils(StructCustomizationUtils.GetPropertyUtilities())
		.SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetTextVisibility))
		.Update();
	TextPropertyRow->OnTextCommittedEvent().AddRaw(this, &Self::HandleTextCommitted);
	TextPropertyRow->OnTextChangedEvent().AddRaw(this, &Self::HandleTextChanged);

	// Text Arguments
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(FDlgEdge::GetMemberNameTextArguments()).ToSharedRef())
		.Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetTextVisibility));

	// Speaker State
	{
		const TSharedPtr<IPropertyHandle> SpeakerStatePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, SpeakerState));

		FDetailWidgetRow* SpeakerStateDetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("SpeakerStateSearchKey", "Speaker State"));

		SpeakerStatePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(SpeakerStateDetailWidgetRow, SpeakerStatePropertyHandle);
		SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.IsEnabled(InStructPropertyHandle->IsEditable())
			.AvailableSuggestions(this, &Self::GetAllDialoguesSpeakerStates)
			.OnTextCommitted(this, &Self::HandleSpeakerStateCommitted)
			.HasContextCheckbox(false)
		)
		.SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetSpeakerStateVisibility))
		.Update();
	}

	// bIncludeInAllOptionListIfUnsatisfied
	IDetailPropertyRow& BoolPropertyRow = StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FDlgEdge, bIncludeInAllOptionListIfUnsatisfied)).ToSharedRef()
	);
	BoolPropertyRow.Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetTextVisibility));
}

void FDialogueEdge_Details::HandleSpeakerStateCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo)
{
	if (Dialogue)
	{
		Dialogue->UpdateAndRefreshData();
	}
}

void FDialogueEdge_Details::HandleTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	if (UDialogueGraphNode_Edge* GraphEdge = FDialogueDetailsPanelUtils::GetAsGraphNodeEdgeFromPropertyHandle(StructPropertyHandle.ToSharedRef()))
	{
		if (Dialogue)
		{
			Dialogue->UpdateAndRefreshData();
		}

		GraphEdge->GetDialogueEdge().RebuildTextArguments();
	}
}

void FDialogueEdge_Details::HandleTextChanged(const FText& InText)
{
	if (UDialogueGraphNode_Edge* GraphEdge = FDialogueDetailsPanelUtils::GetAsGraphNodeEdgeFromPropertyHandle(StructPropertyHandle.ToSharedRef()))
	{
		GraphEdge->GetDialogueEdge().RebuildTextArgumentsFromPreview(InText);
	}
}


#undef LOCTEXT_NAMESPACE
