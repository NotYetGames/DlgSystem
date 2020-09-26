// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueSearchResult.h"

#include "Widgets/Images/SImage.h"
#include "Toolkits/AssetEditorManager.h"

#include "DialogueEditorUtilities.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueStyle.h"

#define LOCTEXT_NAMESPACE "DialogueSearchResult"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchResult
FDialogueSearchResult::FDialogueSearchResult(const FText& InDisplayText, const TSharedPtr<Self>& InParent)
	: Super(InDisplayText, InParent)
{
}

TSharedRef<SWidget>	FDialogueSearchResult::CreateIcon() const
{
	const FLinearColor IconColor = FLinearColor::White;
	const FSlateBrush* Brush = nullptr;

	return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(IconColor)
			.ToolTipText(GetCategory());
}

TWeakObjectPtr<const UDlgDialogue> FDialogueSearchResult::GetParentDialogue() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentDialogue();
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchResult_RootNode
FDialogueSearchResult_RootNode::FDialogueSearchResult_RootNode() :
	Super(FText::FromString(TEXT("Display Text should not be visible")), nullptr)
{
	Category = FText::FromString(TEXT("ROOT NODE SHOULD NOT BE VISIBLE"));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchResult_DialogueNode
FDialogueSearchResult_DialogueNode::FDialogueSearchResult_DialogueNode(const FText& InDisplayText, const TSharedPtr<FDialogueSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
	Category = LOCTEXT("FDialogueSearchResult_DialogueNodeCategory", "Dialogue");
}

FReply FDialogueSearchResult_DialogueNode::OnClick()
{
	if (Dialogue.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorForAsset(Dialogue.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TWeakObjectPtr<const UDlgDialogue> FDialogueSearchResult_DialogueNode::GetParentDialogue() const
{
	// Get the Dialogue from this.
	if (Dialogue.IsValid())
	{
		return Dialogue;
	}

	return Super::GetParentDialogue();
}

TSharedRef<SWidget>	FDialogueSearchResult_DialogueNode::CreateIcon() const
{
	const FSlateBrush* Brush = FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_DlgDialogueClassIcon);

	return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.ToolTipText(GetCategory());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchResult_GraphNode
FDialogueSearchResult_GraphNode::FDialogueSearchResult_GraphNode(const FText& InDisplayText, const TSharedPtr<FDialogueSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
}

FReply FDialogueSearchResult_GraphNode::OnClick()
{
	if (GraphNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(GraphNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> FDialogueSearchResult_GraphNode::CreateIcon() const
{
	if (GraphNode.IsValid())
	{
		FLinearColor Color;
		const FSlateIcon Icon = GraphNode.Get()->GetIconAndTint(Color);
		return SNew(SImage)
				.Image(Icon.GetOptionalIcon())
				.ColorAndOpacity(Color)
				.ToolTipText(GetCategory());
	}

	return Super::CreateIcon();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchResult_EdgeNode
FDialogueSearchResult_EdgeNode::FDialogueSearchResult_EdgeNode(const FText& InDisplayText, const TSharedPtr<FDialogueSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
}

FReply FDialogueSearchResult_EdgeNode::OnClick()
{
	if (EdgeNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(EdgeNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget>	FDialogueSearchResult_EdgeNode::CreateIcon() const
{
	if (EdgeNode.IsValid())
	{
		FLinearColor Color;
		const FSlateIcon Icon = EdgeNode.Get()->GetIconAndTint(Color);
		return SNew(SImage)
				.Image(Icon.GetOptionalIcon())
				.ColorAndOpacity(Color)
				.ToolTipText(GetCategory());
	}

	return Super::CreateIcon();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchResult_CommentNode
FDialogueSearchResult_CommentNode::FDialogueSearchResult_CommentNode(const FText& InDisplayText, const TSharedPtr<FDialogueSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
}

FReply FDialogueSearchResult_CommentNode::OnClick()
{
	if (CommentNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(CommentNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget>	FDialogueSearchResult_CommentNode::CreateIcon() const
{
	if (CommentNode.IsValid())
	{
		const FSlateIcon Icon = FSlateIcon(FDialogueStyle::GetStyleSetName(), FDialogueStyle::PROPERTY_CommentBubbleOn);
		return SNew(SImage)
			.Image(Icon.GetIcon())
			.ColorAndOpacity(FColorList::White)
			.ToolTipText(GetCategory());
	}

	return Super::CreateIcon();
}

#undef LOCTEXT_NAMESPACE
