// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Reply.h"

#include "DlgDialogue.h"
#include "TreeViewHelpers/DlgTreeViewNode.h"

class FFindInDialoguesResult;
class UDialogueGraphNode;
class UDialogueGraphNode_Edge;

typedef TSharedPtr<FFindInDialoguesResult> FFindInDialoguesResultPtr;

/* Base class that matched the search results. When used by itself it is a simple text node. */
class FFindInDialoguesResult : public FDlgTreeViewNode<FFindInDialoguesResult>
{
	typedef FFindInDialoguesResult Self;
	typedef FDlgTreeViewNode Super;

public:
	/* Create a listing for a search result*/
	FFindInDialoguesResult(const FText& InDisplayText, TSharedPtr<Self> InParent);

	/* Create an icon to represent the result */
	virtual TSharedRef<SWidget>	CreateIcon() const;

	/** Gets the Dialogue housing all these search results. Aka the Dialogue this search result belongs to. */
	virtual TWeakObjectPtr<const UDlgDialogue> GetParentDialogue() const;

	// Category:
	FText GetCategory() const { return Category; }
	void SetCategory(const FText& InCategory) { Category = InCategory; }

	// CommentString
	FString GetCommentString() const { return CommentString; }
	void SetCommentString(const FString& InCommentString) { CommentString = InCommentString; }

protected:
	/** The category of this node. */
	FText Category;

	/** Display text for comment information */
	FString CommentString;
};


/** Root Node, should not be displayed. */
class FFindInDialoguesRootNode : public FFindInDialoguesResult
{
	typedef FFindInDialoguesResult Super;
public:
	FFindInDialoguesRootNode();
};


/** Tree Node search results that represents the Dialogue. */
class FFindInDialoguesDialogueNode : public FFindInDialoguesResult
{
	typedef FFindInDialoguesResult Super;
public:
	FFindInDialoguesDialogueNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent);

	FReply OnClick() override;
	TWeakObjectPtr<const UDlgDialogue> GetParentDialogue() const override;
	TSharedRef<SWidget>	CreateIcon() const override;

	// Dialogue:
	void SetDialogue(TWeakObjectPtr<const UDlgDialogue> InDialogue) { Dialogue = InDialogue; }

protected:
	/** The Dialogue this represents. */
	TWeakObjectPtr<const UDlgDialogue> Dialogue;
};


/** Tree Node result that represents the GraphNode */
class FFindInDialoguesGraphNode : public FFindInDialoguesResult
{
	typedef FFindInDialoguesResult Super;
public:
	FFindInDialoguesGraphNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent);

	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;

	// GraphNode:
	void SetGraphNode(TWeakObjectPtr<const UDialogueGraphNode> InGraphNode) { GraphNode = InGraphNode; }

protected:
	/** The GraphNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode> GraphNode;
};


/** Tree Node result that represents the EdgeNode */
class FFindInDialoguesEdgeNode : public FFindInDialoguesResult
{
	typedef FFindInDialoguesResult Super;
public:
	FFindInDialoguesEdgeNode(const FText& InDisplayText, FFindInDialoguesResultPtr InParent);

	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;

	// EdgeNode:
	void SetEdgeNode(TWeakObjectPtr<const UDialogueGraphNode_Edge> InEdgeNode) { EdgeNode = InEdgeNode; }

protected:
	/** The EdgeNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};
