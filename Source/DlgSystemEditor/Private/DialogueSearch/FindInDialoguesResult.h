// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Reply.h"

#include "DlgDialogue.h"

class FFindInDialoguesResult;
class UDialogueGraphNode;
class UDialogueGraphNode_Edge;


typedef TSharedPtr<FFindInDialoguesResult> FFindInDialoguesResultPtr;

/* Base class that matched the search results. When used by itself it is a simple text node. */
class FFindInDialoguesResult : public TSharedFromThis<FFindInDialoguesResult>
{
private:
	typedef FFindInDialoguesResult Self;

public:
	/* Create a root */
	FFindInDialoguesResult(const FText& InDisplayText);

	/* Create a listing for a search result*/
	FFindInDialoguesResult(const FText& InDisplayText, FFindInDialoguesResultPtr InParent);

	virtual ~FFindInDialoguesResult() {}

	/* Called when user clicks on the search item */
	virtual FReply OnClick();

	/* Create an icon to represent the result */
	virtual TSharedRef<SWidget>	CreateIcon() const;

	/** Gets the Dialogue housing all these search results. Aka the Dialogue this search result belongs to. */
	virtual const UDlgDialogue* GetParentDialogue() const;

	/* Get Category for this search result */
	FText GetCategory() const { return Category; }

	/** Returns the display string for the row */
	FText GetDisplayText() const { return DisplayText; }

	/* Gets the comment on this node if any */
	FString GetCommentString() const { return CommentString; }

	/** Gets the parent of this node if any. */
	TWeakPtr<Self> GetParent() const { return Parent; }

	/** Is this the root node? Aka no parent. */
	bool IsRootNode() const { return !Parent.IsValid(); }

	/** Is this the leaft node? Aka no children. */
	bool IsLeafNode() const { return Children.Num() == 0; }

	/**
	 * Takes the tree view and expands its elements for each child.
	 *
	 * @param  TreeView		The tree responsible for visualizing this node hierarchy.
	 * @param  bRecursive	Determines if you want children/descendants to expand their children as well.
	 */
	void ExpandAllChildren(TSharedPtr<STreeView<FFindInDialoguesResultPtr>> TreeView, bool bRecursive = true);

public:
	/** Any children listed under this node. */
	TArray<FFindInDialoguesResultPtr> Children;

	/** The node that this is a direct child of (empty if this is a root node)  */
	TWeakPtr<Self> Parent;

	/** The display text for this item */
	FText DisplayText;

	/** The category of this node. */
	FText Category;

	/** Display text for comment information */
	FString CommentString;
};

/** Root Node, should not be displayed. */
class FFindInDialoguesRootNode : public FFindInDialoguesResult
{
public:
	FFindInDialoguesRootNode();
};

/** Tree Node search results that represents the Dialogue. */
class FFindInDialoguesDialogueNode : public FFindInDialoguesResult
{
private:
	typedef FFindInDialoguesResult Super;

public:
	FFindInDialoguesDialogueNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent);

	/** FFindInDialoguesResult interface */
	FReply OnClick() override;
	const UDlgDialogue* GetParentDialogue() const override;
	TSharedRef<SWidget>	CreateIcon() const override;
	/** End FFindInDialoguesResult interface */

public:
	/** The Dialogue this represents. */
	TWeakObjectPtr<const UDlgDialogue> Dialogue;
};

/** Tree Node result that represents the GraphNode */
class FFindInDialoguesGraphNode : public FFindInDialoguesResult
{
private:
	typedef FFindInDialoguesResult Super;

public:
	FFindInDialoguesGraphNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent);

	/** FFindInDialoguesResult interface */
	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;
	/** End FFindInDialoguesResult interface */
public:
	/** The GraphNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode> GraphNode;
};

/** Tree Node result that represents the EdgeNode */
class FFindInDialoguesEdgeNode : public FFindInDialoguesResult
{
private:
	typedef FFindInDialoguesResult Super;

public:
	FFindInDialoguesEdgeNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent);

	/** FFindInDialoguesResult interface */
	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;
	/** End FFindInDialoguesResult interface */
public:
	/** The GraphNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};
