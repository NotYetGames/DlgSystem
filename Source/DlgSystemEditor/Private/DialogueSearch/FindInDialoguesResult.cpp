// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "FindInDialoguesResult.h"

#include "Widgets/Images/SImage.h"
#include "Toolkits/AssetEditorManager.h"

#include "DialogueEditor/DialogueEditorUtilities.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueStyle.h"

#define LOCTEXT_NAMESPACE "FFindInDialoguesResult"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialoguesResult
FFindInDialoguesResult::FFindInDialoguesResult(const FText& InDisplayText, const TSharedPtr<Self>& InParent)
	: Super(InDisplayText, InParent)
{
}

TSharedRef<SWidget>	FFindInDialoguesResult::CreateIcon() const
{
	const FLinearColor IconColor = FLinearColor::White;
	const FSlateBrush* Brush = nullptr;

	return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(IconColor)
			.ToolTipText(GetCategory());
}

TWeakObjectPtr<const UDlgDialogue> FFindInDialoguesResult::GetParentDialogue() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentDialogue();
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialoguesRootNode
FFindInDialoguesRootNode::FFindInDialoguesRootNode() :
	Super(FText::FromString(TEXT("Display Text should not be visible")), nullptr)
{
	Category = LOCTEXT("FFindInDialoguesRootNodeCategory", "ROOT NODE SHOULD NOT BE VISIBLE");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialoguesDialogueNode
FFindInDialoguesDialogueNode::FFindInDialoguesDialogueNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent) :
	Super(InDisplayText, InParent)
{
	Category = LOCTEXT("FFindInDialoguesDialogueNodeCategory", "Dialogue");
}

FReply FFindInDialoguesDialogueNode::OnClick()
{
	if (Dialogue.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorForAsset(Dialogue.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TWeakObjectPtr<const UDlgDialogue> FFindInDialoguesDialogueNode::GetParentDialogue() const
{
	// Get the Dialogue from this.
	if (Dialogue.IsValid())
	{
		return Dialogue;
	}

	return Super::GetParentDialogue();
}

TSharedRef<SWidget>	FFindInDialoguesDialogueNode::CreateIcon() const
{
	const FSlateBrush* Brush = FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_DialogueClassIcon);

	return SNew(SImage)
			.Image(Brush)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.ToolTipText(GetCategory());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialoguesGraphNode
FFindInDialoguesGraphNode::FFindInDialoguesGraphNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent) :
	Super(InDisplayText, InParent)
{
}

FReply FFindInDialoguesGraphNode::OnClick()
{
	if (GraphNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(GraphNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> FFindInDialoguesGraphNode::CreateIcon() const
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
// FFindInDialoguesEdgeNode
FFindInDialoguesEdgeNode::FFindInDialoguesEdgeNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent) :
	FFindInDialoguesResult(InDisplayText, InParent)
{
}

FReply FFindInDialoguesEdgeNode::OnClick()
{
	if (EdgeNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(EdgeNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget>	FFindInDialoguesEdgeNode::CreateIcon() const
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
// FFindInDialoguesCommentNode
FFindInDialoguesCommentNode::FFindInDialoguesCommentNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent) :
	FFindInDialoguesResult(InDisplayText, InParent)
{
}

FReply FFindInDialoguesCommentNode::OnClick()
{
	if (CommentNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(CommentNode.Get()) ? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget>	FFindInDialoguesCommentNode::CreateIcon() const
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
