// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgDataDisplayTreeNode.h"

#define LOCTEXT_NAMESPACE "FDlgDataDisplayTreeNode"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgDataDisplayTreeNode
FDlgDataDisplayTreeNode::FDlgDataDisplayTreeNode(const FText& InDisplayText, TSharedPtr<Self> InParent)
	: DisplayText(InDisplayText), Parent(InParent)
{
}

TWeakObjectPtr<const AActor> FDlgDataDisplayTreeNode::GetParentActor() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentActor();
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeRootNode
FDlgDataDisplayTreeRootNode::FDlgDataDisplayTreeRootNode() :
	Super(FText::FromString(TEXT("ROOT")), nullptr)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgDataDisplayTreeActorNode
FDlgDataDisplayTreeActorNode::FDlgDataDisplayTreeActorNode(const FText& InDisplayText,
	FDlgDataDisplayTreeNodePtr InParent, TWeakObjectPtr<const AActor> InActor) :
	Super(InDisplayText, InParent), Actor(InActor)
{
}

TWeakObjectPtr<const AActor> FDlgDataDisplayTreeActorNode::GetParentActor() const
{
	// Get the Actor from this.
	if (Actor.IsValid())
	{
		return Actor;
	}

	return Super::GetParentActor();
}

#undef LOCTEXT_NAMESPACE
