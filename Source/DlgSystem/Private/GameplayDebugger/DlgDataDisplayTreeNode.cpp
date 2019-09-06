// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "GameplayDebugger/DlgDataDisplayTreeNode.h"

#define LOCTEXT_NAMESPACE "FDlgDataDisplayTreeNode"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgDataDisplayTreeNode
FDlgDataDisplayTreeNode::FDlgDataDisplayTreeNode(const FText& InDisplayText, const TSharedPtr<Self>& InParent)
	: Super(InDisplayText, InParent)
{
}

TWeakObjectPtr<AActor> FDlgDataDisplayTreeNode::GetParentActor() const
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
	const TSharedPtr<FDlgDataDisplayTreeNode>& InParent, TWeakObjectPtr<AActor> InActor) :
	Super(InDisplayText, InParent), Actor(InActor)
{
	TextType = EDlgDataDisplayTextTreeNodeType::Actor;
}

TWeakObjectPtr<AActor> FDlgDataDisplayTreeActorNode::GetParentActor() const
{
	// Get the Actor from this.
	if (Actor.IsValid())
	{
		return Actor;
	}

	return Super::GetParentActor();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgDataDisplayTreeCategoryNode
FDlgDataDisplayTreeCategoryNode::FDlgDataDisplayTreeCategoryNode(const FText& InDisplayText,
	const TSharedPtr<FDlgDataDisplayTreeNode>& InParent, const EDlgDataDisplayCategoryTreeNodeType InCategoryType) :
	Super(InDisplayText, InParent)
{
	CategoryType = InCategoryType;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgDataDisplayTreeVariableNode
FDlgDataDisplayTreeVariableNode::FDlgDataDisplayTreeVariableNode(const FText& InDisplayText,
	const TSharedPtr<FDlgDataDisplayTreeNode>& InParent, const FName& InVariableName,
	const EDlgDataDisplayVariableTreeNodeType InVariableType) :
	Super(InDisplayText, InParent), VariableName(InVariableName), VariableType(InVariableType)
{
	TextType = EDlgDataDisplayTextTreeNodeType::Variable;
}

#undef LOCTEXT_NAMESPACE
