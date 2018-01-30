// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Reply.h"
#include "UObject/WeakObjectPtr.h"
#include "Containers/Set.h"
#include "GameFramework/Actor.h"

#include "TreeViewHelpers/DlgTreeViewNode.h"

class FDlgDataDisplayTreeNode;
typedef TSharedPtr<FDlgDataDisplayTreeNode> FDlgDataDisplayTreeNodePtr;

/* Base class node for all Nodes in the DlgDataDisplayWindow  */
class FDlgDataDisplayTreeNode : public FDlgTreeViewNode<FDlgDataDisplayTreeNode>
{
	typedef FDlgDataDisplayTreeNode Self;
	typedef FDlgTreeViewNode Super;
public:
	FDlgDataDisplayTreeNode(const FText& InDisplayText, TSharedPtr<Self> InParent);

	/** Gets the Actor that has this Node belongs to. */
	virtual TWeakObjectPtr<const AActor> GetParentActor() const;
};


/** Root node of the Dialogue Data Display */
class FDlgDataDisplayTreeRootNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeRootNode();
};


/** Bide result that Represents the Actor. */
class FDlgDataDisplayTreeActorNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeActorNode(const FText& InDisplayText, FDlgDataDisplayTreeNodePtr InParent,
		TWeakObjectPtr<const AActor> InActor);

	/** FDlgDataDisplayTreeNode interface */
	TWeakObjectPtr<const AActor> GetParentActor() const override;
	/** End FDlgDataDisplayTreeNode interface */

protected:
	/** The Actor this represents. */
	TWeakObjectPtr<const AActor> Actor;
};
