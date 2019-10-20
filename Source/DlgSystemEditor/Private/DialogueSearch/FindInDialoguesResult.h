// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"

#include "DlgDialogue.h"
#include "TreeViewHelpers/DlgTreeViewNode.h"

class FFindInDialoguesResult;
class UDialogueGraphNode;
class UDialogueGraphNode_Edge;
class UEdGraphNode_Comment;

// Filter used when searching for Dialogue Data
struct FDialogueSearchFilter
{
public:
	bool IsEmptyFilter() const
	{
		return SearchString.IsEmpty()
			&& bIncludeIndices == false
			&& bIncludeDialogueGUID == false
			&& bIncludeComments == true
			&& bIncludeNumericalTypes == false;
	}

public:
	// Search term that the search items must match
	FString SearchString;

	// Include node/edge indices in search results?
	bool bIncludeIndices = false;

	// Include the Dialogue GUID in search results
	bool bIncludeDialogueGUID = false;

	// Include node comments in search results?
	bool bIncludeComments = true;

	// Include numerical data in search results like (int32, floats)?
	bool bIncludeNumericalTypes = false;

	// Include the Text localization data in search results (namespace, key)
	bool bIncludeTextLocalizationData = false;
};

/* Base class that matched the search results. When used by itself it is a simple text node. */
class FFindInDialoguesResult : public FDlgTreeViewNode<FFindInDialoguesResult>
{
	typedef FFindInDialoguesResult Self;
	typedef FDlgTreeViewNode Super;

public:
	/* Create a listing for a search result*/
	FFindInDialoguesResult(const FText& InDisplayText, const TSharedPtr<Self>& InParent);

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
	FFindInDialoguesDialogueNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent);

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
	FFindInDialoguesGraphNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent);

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
	FFindInDialoguesEdgeNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent);

	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;

	// EdgeNode:
	void SetEdgeNode(TWeakObjectPtr<const UDialogueGraphNode_Edge> InEdgeNode) { EdgeNode = InEdgeNode; }

protected:
	/** The EdgeNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};

/** Tree Node result that represents the CommentNode */
class FFindInDialoguesCommentNode : public FFindInDialoguesResult
{
	typedef FFindInDialoguesResult Super;
public:
	FFindInDialoguesCommentNode(const FText& InDisplayText, const TSharedPtr<FFindInDialoguesResult>& InParent);

	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;

	// CommentNode:
	void SetCommentNode(TWeakObjectPtr<const UEdGraphNode_Comment> InCommentNode) { CommentNode = InCommentNode; }

protected:
	/** The EdgeNode this represents. */
	TWeakObjectPtr<const UEdGraphNode_Comment> CommentNode;
};
