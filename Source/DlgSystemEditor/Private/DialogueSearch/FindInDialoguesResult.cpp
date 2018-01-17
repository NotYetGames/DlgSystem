// Fill out your copyright notice in the Description page of Project Settings.
#include "FindInDialoguesResult.h"

#include "SImage.h"
#include "STreeView.h"
#include "AssetEditorManager.h"

#include "DialogueEditor/DialogueEditorUtilities.h"
#include "DialogueStyle.h"

#define LOCTEXT_NAMESPACE "FFindInDialoguesResult"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialoguesResult
FFindInDialoguesResult::FFindInDialoguesResult(const FText& InDisplayText)
	: DisplayText(InDisplayText)
{
}

FFindInDialoguesResult::FFindInDialoguesResult(const FText& InDisplayText, FFindInDialoguesResultPtr InParent)
	: Parent(InParent), DisplayText(InDisplayText)
{
}

/* Called when user clicks on the search item */
FReply FFindInDialoguesResult::OnClick()
{
	// If there is a parent, handle it using the parent's functionality
	if (Parent.IsValid())
	{
		return Parent.Pin()->OnClick();
	}

	return FReply::Unhandled();
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

const UDlgDialogue* FFindInDialoguesResult::GetParentDialogue() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentDialogue();
	}

	return nullptr;
}

void FFindInDialoguesResult::ExpandAllChildren(TSharedPtr<STreeView<FFindInDialoguesResultPtr>> TreeView,
												bool bRecursive /*= true*/)
{
	static constexpr bool bShouldExpandItem = true;

	if (Children.Num() == 0)
	{
		return;
	}

	TreeView->SetItemExpansion(this->AsShared(), bShouldExpandItem);
	for (FFindInDialoguesResultPtr& ChildNode : Children)
	{
		if (bRecursive)
		{
			// recursive on all children.
			ChildNode->ExpandAllChildren(TreeView, bRecursive);
		}
		else
		{
			// Only direct children
			TreeView->SetItemExpansion(ChildNode, bShouldExpandItem);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialoguesRootNode
FFindInDialoguesRootNode::FFindInDialoguesRootNode() :
	FFindInDialoguesResult(FText::FromString(TEXT("Display Text should not be visible")), nullptr)
{
	Category = LOCTEXT("FFindInDialoguesRootNodeCategory", "ROOT NODE SHOULD NOT BE VISIBLE");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialoguesDialogueNode
FFindInDialoguesDialogueNode::FFindInDialoguesDialogueNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent) :
	FFindInDialoguesResult(InDisplayText, InParent)
{
	Category = LOCTEXT("FFindInDialoguesDialogueNodeCategory", "Dialogue");
}

FReply FFindInDialoguesDialogueNode::OnClick()
{
	if (Dialogue.IsValid())
	{
		FAssetEditorManager::Get().OpenEditorForAsset(const_cast<UDlgDialogue*>(Dialogue.Get()));
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

const UDlgDialogue* FFindInDialoguesDialogueNode::GetParentDialogue() const
{
	// Get the Dialogue from this.
	if (Dialogue.IsValid())
	{
		return Dialogue.Get();
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
FFindInDialoguesGraphNode::FFindInDialoguesGraphNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent) :
	FFindInDialoguesResult(InDisplayText, InParent)
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
FFindInDialoguesEdgeNode::FFindInDialoguesEdgeNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent) :
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

#undef LOCTEXT_NAMESPACE
