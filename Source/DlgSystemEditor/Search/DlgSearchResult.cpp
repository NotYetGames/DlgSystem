// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgSearchResult.h"

#include "Widgets/Images/SImage.h"

#include "DlgSystemEditor/DlgEditorUtilities.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgSystemEditor/DlgStyle.h"

#define LOCTEXT_NAMESPACE "DialogueSearchResult"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgSearchResult
FDlgSearchResult::FDlgSearchResult(const FText& InDisplayText, const TSharedPtr<Self>& InParent)
	: Super(InDisplayText, InParent)
{
}

TSharedRef<SWidget>	FDlgSearchResult::CreateIcon() const
{
	const FLinearColor IconColor = FLinearColor::White;
	const FSlateBrush* Brush = nullptr;

	return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(IconColor)
			.ToolTipText(GetCategory());
}

TWeakObjectPtr<const UDlgDialogue> FDlgSearchResult::GetParentDialogue() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentDialogue();
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgSearchResult_RootNode
FDlgSearchResult_RootNode::FDlgSearchResult_RootNode() :
	Super(FText::FromString(TEXT("Display Text should not be visible")), nullptr)
{
	Category = FText::FromString(TEXT("ROOT NODE SHOULD NOT BE VISIBLE"));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgSearchResult_DialogueNode
FDlgSearchResult_DialogueNode::FDlgSearchResult_DialogueNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
	Category = LOCTEXT("FDialogueSearchResult_DialogueNodeCategory", "Dialogue");
}

FReply FDlgSearchResult_DialogueNode::OnClick()
{
	if (Dialogue.IsValid())
	{
		return FDlgEditorUtilities::OpenEditorForAsset(Dialogue.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TWeakObjectPtr<const UDlgDialogue> FDlgSearchResult_DialogueNode::GetParentDialogue() const
{
	// Get the Dialogue from this.
	if (Dialogue.IsValid())
	{
		return Dialogue;
	}

	return Super::GetParentDialogue();
}

TSharedRef<SWidget>	FDlgSearchResult_DialogueNode::CreateIcon() const
{
	const FSlateBrush* Brush = FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_DlgDialogueClassIcon);

	return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.ToolTipText(GetCategory());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgSearchResult_GraphNode
FDlgSearchResult_GraphNode::FDlgSearchResult_GraphNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
}

FReply FDlgSearchResult_GraphNode::OnClick()
{
	if (GraphNode.IsValid())
	{
		return FDlgEditorUtilities::OpenEditorAndJumpToGraphNode(GraphNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> FDlgSearchResult_GraphNode::CreateIcon() const
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
// FDlgSearchResult_EdgeNode
FDlgSearchResult_EdgeNode::FDlgSearchResult_EdgeNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
}

FReply FDlgSearchResult_EdgeNode::OnClick()
{
	if (EdgeNode.IsValid())
	{
		return FDlgEditorUtilities::OpenEditorAndJumpToGraphNode(EdgeNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget>	FDlgSearchResult_EdgeNode::CreateIcon() const
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
// FDlgSearchResult_CommentNode
FDlgSearchResult_CommentNode::FDlgSearchResult_CommentNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent) :
	Super(InDisplayText, InParent)
{
}

FReply FDlgSearchResult_CommentNode::OnClick()
{
	if (CommentNode.IsValid())
	{
		return FDlgEditorUtilities::OpenEditorAndJumpToGraphNode(CommentNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget>	FDlgSearchResult_CommentNode::CreateIcon() const
{
	if (CommentNode.IsValid())
	{
		const FSlateIcon Icon = FSlateIcon(FDlgStyle::GetStyleSetName(), FDlgStyle::PROPERTY_CommentBubbleOn);
		return SNew(SImage)
			.Image(Icon.GetIcon())
			.ColorAndOpacity(FColorList::White)
			.ToolTipText(GetCategory());
	}

	return Super::CreateIcon();
}

#undef LOCTEXT_NAMESPACE
