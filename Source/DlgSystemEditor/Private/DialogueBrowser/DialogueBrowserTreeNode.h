// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "STreeView.h"

#include "TreeViewHelpers/DlgTreeViewNode.h"

class FDialogueBrowserTreeNode;
class UDlgDialogue;
class UDialogueGraphNode;
class UDialogueGraphNode_Edge;

typedef TSharedPtr<FDialogueBrowserTreeNode> FDialogueBrowserTreeNodePtr;

/** The types of categories. */
enum class EDialogueTreeNodeCategoryType : uint8
{
	Default = 0,

	Participant,
	Dialogue,
	Event,
	Condition,
	Variable,
	VariableInt,
	VariableFloat,
	VariableBool,
	VariableFName,

	Max
};

enum class EDialogueTreeNodeTextType : uint8
{
	Default = 0,

	ParticipantDialogue,
	ParticipantEvent,
	ParticipantCondition,
	ParticipantVariableInt,
	ParticipantVariableFloat,
	ParticipantVariableBool,
	ParticipantVariableFName,

	EventDialogue,
	EventGraphNode,

	ConditionDialogue,
	ConditionGraphNode,
	ConditionEdgeNode,

	IntVariableDialogue,
	IntVariableGraphNode,
	IntVariableEdgeNode,

	FloatVariableDialogue,
	FloatVariableGraphNode,
	FloatVariableEdgeNode,

	BoolVariableDialogue,
	BoolVariableGraphNode,
	BoolVariableEdgeNode,

	FNameVariableDialogue,
	FNameVariableGraphNode,
	FNameVariableEdgeNode,

	Max
};

/** Defines the type of the DialogueTree node. */
enum class EDialogueTreeNodeType : uint8
{
	Text = 0,
	Separator,
	Category,

	Max
};

/**
 * Defines the singular item in the Tree.
 */
class FDialogueBrowserTreeNode : public FDlgTreeViewNode<FDialogueBrowserTreeNode>
{
	typedef FDialogueBrowserTreeNode Self;
	typedef FDlgTreeViewNode Super;

public:
	FDialogueBrowserTreeNode(const FText& InDisplayText, TSharedPtr<Self> InParent);

	/** Gets the Participant Name that has this Node belongs to */
	virtual const FName GetParentParticipantName() const;

	/** Getters for the properties */

	// VariableName:
	const FName& GetVariableName() const { return VariableName; }
	void SetVariableName(const FName& InVariableName) { VariableName = InVariableName; }

	// Type:
	EDialogueTreeNodeType GetType() const { return Type; }

	// TextType:
	EDialogueTreeNodeTextType GetTextType() const { return TextType; }
	void SetTextType(EDialogueTreeNodeTextType InTextType) { TextType = InTextType; }

	// CategoryType:
	EDialogueTreeNodeCategoryType GetCategoryType() const { return CategoryType; }
	void SetCategoryType(EDialogueTreeNodeCategoryType InCategoryType) { CategoryType = InCategoryType; }

	// Children/InlineChildren:
	void AddChild(const TSharedPtr<Self>& ChildNode) override
	{
		ensure(!IsSeparator());
		Super::AddChild(ChildNode);
	}
	void ClearChildren() override
	{
		Super::ClearChildren();
		InlineChildren.Empty();
	}

	void AddInlineChild(const TSharedPtr<Self>& ChildNode, const bool bIsInline = false)
	{
		ensure(!ChildNode->IsRoot());
		ensure(!IsSeparator());
		ChildNode->SetParent(this->AsShared());
		InlineChildren.Add(ChildNode);
	}
	bool HasInlineChildren() const { return InlineChildren.Num() > 0; }
	const TArray<TSharedPtr<Self>>& GetInlineChildren() const { return InlineChildren; }
	void SetInlineChildren(const TArray<TSharedPtr<Self>>& InChildren)
	{
		InlineChildren = InChildren;
		for (const TSharedPtr<Self>& Child : InlineChildren)
		{
			Child->SetParent(this->AsShared());
		}
	}

	/** Checks type of this Node. */
	bool IsText() const { return Type == EDialogueTreeNodeType::Text; }
	bool IsCategory() const { return Type == EDialogueTreeNodeType::Category; }
	bool IsSeparator() const { return Type == EDialogueTreeNodeType::Separator; }
	bool IsDialogueText() const
	{
		return IsText() && (TextType == EDialogueTreeNodeTextType::ParticipantDialogue ||
							TextType == EDialogueTreeNodeTextType::EventDialogue ||
							TextType == EDialogueTreeNodeTextType::ConditionDialogue ||
							TextType == EDialogueTreeNodeTextType::IntVariableDialogue ||
							TextType == EDialogueTreeNodeTextType::FloatVariableDialogue ||
							TextType == EDialogueTreeNodeTextType::BoolVariableDialogue ||
							TextType == EDialogueTreeNodeTextType::FNameVariableDialogue);
	}
	bool IsEventText() const
	{
		return IsText() && (TextType == EDialogueTreeNodeTextType::ParticipantEvent);
	}
	bool IsConditionText() const
	{
		return IsText() && (TextType == EDialogueTreeNodeTextType::ParticipantCondition);
	}
	bool IsGraphNodeText() const
	{
		return IsText() && (TextType == EDialogueTreeNodeTextType::EventGraphNode ||
							TextType == EDialogueTreeNodeTextType::ConditionGraphNode ||
							TextType == EDialogueTreeNodeTextType::IntVariableGraphNode ||
							TextType == EDialogueTreeNodeTextType::FloatVariableGraphNode ||
							TextType == EDialogueTreeNodeTextType::BoolVariableGraphNode ||
							TextType == EDialogueTreeNodeTextType::FNameVariableGraphNode);
	}
	bool IsEdgeNodeText()
	{
		return IsText() && (TextType == EDialogueTreeNodeTextType::ConditionEdgeNode ||
							TextType == EDialogueTreeNodeTextType::IntVariableEdgeNode ||
							TextType == EDialogueTreeNodeTextType::FloatVariableEdgeNode ||
							TextType == EDialogueTreeNodeTextType::BoolVariableEdgeNode ||
							TextType == EDialogueTreeNodeTextType::FNameVariableEdgeNode);
	}

	/**
	 * Filters the node so that it will only containt paths to nodes that contains the specified string.
	 * @param OutNodes	Array of arrays, each array inside represents a node path that remains to the Node that contains the InSearch
	 * @param InSearch The string to search by
	 */
	void FilterPathsToNodesThatContainText(const FString& InSearch, TArray<TArray<TSharedPtr<Self>>>& OutNodes);

	/** Gets the textual representation of this item */
	FString ToString() const;


	/** Is this equal with Other? */
	virtual bool IsEqual(const Self& Other)
	{
		return 	DisplayText.EqualTo(Other.GetDisplayText()) &&
				VariableName == Other.GetVariableName() &&
				Type == Other.GetType() &&
				CategoryType == Other.GetCategoryType() &&
				TextType == Other.GetTextType();
	}

	bool operator==(const Self& Other)
	{
		return IsEqual(Other);
	}

private:
	void GetPathToChildThatContainsText(const TSharedPtr<Self>& Child,
										const FString& InSearch,
										TArray<TArray<TSharedPtr<Self>>>& OutNodes);

protected:
	// Used to store Event, Condition, IntName, Dialogue name etc
	FName VariableName = NAME_None;

	/** The type of this item. */
	EDialogueTreeNodeType Type = EDialogueTreeNodeType::Text;

	// Specific category type, only used if Type is Category.
	EDialogueTreeNodeCategoryType CategoryType = EDialogueTreeNodeCategoryType::Default;

	// Specific text type, only used if the Type is Text.
	EDialogueTreeNodeTextType TextType = EDialogueTreeNodeTextType::Default;

	// Inline Nodes, Nodes that are displayed in the same line as this Node
	TArray<TSharedPtr<Self>> InlineChildren;
};


/** Root node of the Dialogue browser */
class FDialogueBrowserTreeRootNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeRootNode();
};


/** Separator node of the Dialogue browser */
class FDialogueBrowserTreeSeparatorNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeSeparatorNode(FDialogueBrowserTreeNodePtr InParent = nullptr);
};


/**
 * Category node of the Dialogue browser. Defines a node that is a Category.
 * The same as FDialogueBrowserTreeNode only that is of type Text.
 */
class FDialogueBrowserTreeCategoryNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeCategoryNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
									const EDialogueTreeNodeCategoryType InCategoryType);
};


/** Node results that represents the Participant Name. */
class FDialogueBrowserTreeParticipantNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeParticipantNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeParticipantNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
										const FName& InParticipantName);

	// ParticipantName:
	const FName GetParentParticipantName() const override;
	void SetParticipantName(const FName& InParticipantName) { ParticipantName = InParticipantName; }

	bool operator==(const Self& Other)
	{
		return ParticipantName == Other.GetParentParticipantName() &&
			   IsEqual(Other);
	 }

protected:
	/** The Participant Name it represents. */
	FName ParticipantName = NAME_None;
};


/** Similar to the FDialogueBrowserTreeParticipantNode only this is a Category */
class FDialogueBrowserTreeCategoryParticipantNode : public FDialogueBrowserTreeParticipantNode
{
	typedef FDialogueBrowserTreeParticipantNode Super;
public:
	FDialogueBrowserTreeCategoryParticipantNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
		const FName& InParticipantName);
};


/** Node results that represents the Dialogue. */
class FDialogueBrowserTreeDialogueNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeDialogueNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
									const TWeakObjectPtr<const UDlgDialogue>& InObject);

	// Dialogue:
	const TWeakObjectPtr<const UDlgDialogue>& GetDialogue() const { return Dialogue; }
	FReply OnClick() override;

protected:
	/** The Dialogue this represents. */
	TWeakObjectPtr<const UDlgDialogue> Dialogue;
};


/** Node results that represents the GraphNode. */
class FDialogueBrowserTreeGraphNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeGraphNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
								const TWeakObjectPtr<const UDialogueGraphNode>& InObject);

	// GraphNode:
	const TWeakObjectPtr<const UDialogueGraphNode>& GetGraphNode() const { return GraphNode; }
	FReply OnClick() override;

protected:
	/** The GraphNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode> GraphNode;
};


/** Node results that represents the EdgeNode. */
class FDialogueBrowserTreeEdgeNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeEdgeNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
								const TWeakObjectPtr<const UDialogueGraphNode_Edge>& InObject);

	// EdgeNode:
	const TWeakObjectPtr<const UDialogueGraphNode_Edge>& GetEdgeNode() const { return EdgeNode; }
	FReply OnClick() override;

protected:
	/** The EdgeNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};
