// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"

#include "DlgSystem/DlgDialogue.h"
#include "DlgSystem/TreeViewHelpers/DlgTreeViewNode.h"

class FDlgSearchResult;
class UDialogueGraphNode;
class UDialogueGraphNode_Edge;
class UEdGraphNode_Comment;

// Filter used when searching for Dialogue Data
struct DLGSYSTEMEDITOR_API FDlgSearchFilter
{
public:
	bool IsEmptyFilter() const
	{
		return SearchString.IsEmpty()
			&& bIncludeIndices == false
			&& bIncludeDialogueGUID == false
			&& bIncludeNodeGUID == false
			&& bIncludeComments == true
			&& bIncludeNumericalTypes == false
			&& bIncludeCustomObjectNames == false;
	}

public:
	// Search term that the search items must match
	FString SearchString;

	// Include node/edge indices in search results?
	bool bIncludeIndices = false;

	// Include the Dialogue GUID in search results
	bool bIncludeDialogueGUID = false;

	// Include the Node GUID in search results
	bool bIncludeNodeGUID = false;

	// Include node comments in search results?
	bool bIncludeComments = true;

	// Include numerical data in search results like (int32, floats)?
	bool bIncludeNumericalTypes = false;

	// Include the Text localization data in search results (namespace, key)
	bool bIncludeTextLocalizationData = false;

	// Include the Custom Text Argument/Condition/Event/Node Data object names
	bool bIncludeCustomObjectNames = true;
};

// Base class that matched the search results. When used by itself it is a simple text node.
class DLGSYSTEMEDITOR_API FDlgSearchResult : public FDlgTreeViewNode<FDlgSearchResult>
{
	typedef FDlgSearchResult Self;
	typedef FDlgTreeViewNode Super;

public:
	FDlgSearchResult(const FText& InDisplayText, const TSharedPtr<Self>& InParent);

	// Create an icon to represent the result
	virtual TSharedRef<SWidget>	CreateIcon() const;

	// Gets the Dialogue housing all these search results. Aka the Dialogue this search result belongs to.
	virtual TWeakObjectPtr<const UDlgDialogue> GetParentDialogue() const;

	// Category:
	FText GetCategory() const { return Category; }
	void SetCategory(const FText& InCategory) { Category = InCategory; }

	// CommentString
	FString GetCommentString() const { return CommentString; }
	void SetCommentString(const FString& InCommentString) { CommentString = InCommentString; }

protected:
	// The category of this node.
	FText Category;

	// Display text for comment information
	FString CommentString;
};


// Root Node, should not be displayed.
class DLGSYSTEMEDITOR_API FDlgSearchResult_RootNode : public FDlgSearchResult
{
	typedef FDlgSearchResult Super;
public:
	FDlgSearchResult_RootNode();
};


// Tree Node search results that represents the Dialogue.
class DLGSYSTEMEDITOR_API FDlgSearchResult_DialogueNode : public FDlgSearchResult
{
	typedef FDlgSearchResult Super;
public:
	FDlgSearchResult_DialogueNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent);

	FReply OnClick() override;
	TWeakObjectPtr<const UDlgDialogue> GetParentDialogue() const override;
	TSharedRef<SWidget>	CreateIcon() const override;

	// Dialogue:
	void SetDialogue(TWeakObjectPtr<const UDlgDialogue> InDialogue) { Dialogue = InDialogue; }

protected:
	// The Dialogue this represents.
	TWeakObjectPtr<const UDlgDialogue> Dialogue;
};


// Tree Node result that represents the GraphNode
class DLGSYSTEMEDITOR_API FDlgSearchResult_GraphNode : public FDlgSearchResult
{
	typedef FDlgSearchResult Super;
public:
	FDlgSearchResult_GraphNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent);

	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;

	// GraphNode:
	void SetGraphNode(TWeakObjectPtr<const UDialogueGraphNode> InGraphNode) { GraphNode = InGraphNode; }

protected:
	// The GraphNode this represents.
	TWeakObjectPtr<const UDialogueGraphNode> GraphNode;
};


// Tree Node result that represents the EdgeNode
class DLGSYSTEMEDITOR_API FDlgSearchResult_EdgeNode : public FDlgSearchResult
{
	typedef FDlgSearchResult Super;
public:
	FDlgSearchResult_EdgeNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent);

	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;

	// EdgeNode:
	void SetEdgeNode(TWeakObjectPtr<const UDialogueGraphNode_Edge> InEdgeNode) { EdgeNode = InEdgeNode; }

protected:
	// The EdgeNode this represents.
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};

// Tree Node result that represents the CommentNode
class DLGSYSTEMEDITOR_API FDlgSearchResult_CommentNode : public FDlgSearchResult
{
	typedef FDlgSearchResult Super;
public:
	FDlgSearchResult_CommentNode(const FText& InDisplayText, const TSharedPtr<FDlgSearchResult>& InParent);

	FReply OnClick() override;
	TSharedRef<SWidget> CreateIcon() const override;

	// CommentNode:
	void SetCommentNode(TWeakObjectPtr<const UEdGraphNode_Comment> InCommentNode) { CommentNode = InCommentNode; }

protected:
	// The EdgeNode this represents.
	TWeakObjectPtr<const UEdGraphNode_Comment> CommentNode;
};
