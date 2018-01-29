// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "STreeView.h"

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
class FDialogueBrowserTreeNode : public  TSharedFromThis<FDialogueBrowserTreeNode>
{
	typedef FDialogueBrowserTreeNode Self;

public:
	FDialogueBrowserTreeNode(const FName& InText, TSharedPtr<Self> InParent);
	virtual ~FDialogueBrowserTreeNode() {}

	/** Gets the Participant Name that has this Node belongs to */
	virtual const FName GetParentParticipantName() const;

	/* Called when user clicks on the search item */
	virtual FReply OnClick();

	/** Getters for the properties */
	// Text:
	const FName& GetText() const { return Text; }
	FText GetTextAsFText() const { return FText::FromName(Text); }
	FString GetTextAsFString() const { return Text.ToString(); }
	Self* SetText(const FName& InText)
	{
		Text = InText;
		return this;
	}
	bool TextContains(const FString& InSearch, ESearchCase::Type SearchCase = ESearchCase::IgnoreCase)
	{
		return Text.ToString().Contains(InSearch, SearchCase);
	}

	// VariableName:
	const FName& GetVariableName() const { return VariableName; }
	Self* SetVariableName(const FName& InVariableName)
	{
		VariableName = InVariableName;
		return this;
	}

	// Type:
	EDialogueTreeNodeType GetType() const { return Type; }

	// TextType:
	EDialogueTreeNodeTextType GetTextType() const { return TextType; }
	Self* SetTextType(EDialogueTreeNodeTextType InTextType)
	{
		TextType = InTextType;
		return this;
	}

	// CategoryType:
	EDialogueTreeNodeCategoryType GetCategoryType() const { return CategoryType; }
	Self* SetCategoryType(EDialogueTreeNodeCategoryType InCategoryType)
	{
		CategoryType = InCategoryType;
		return this;
	}

	// bIsVisible:
	bool IsVisible() const { return bIsVisible; }
	Self* SetIsVisible(const bool InIsVisible)
	{
		bIsVisible = InIsVisible;
		return this;
	}

	// ParentNode:
	bool HasParent() const { return Parent.IsValid(); }
	TWeakPtr<Self> GetParent() const { return Parent; }
	Self* SetParent(TWeakPtr<Self> InParentNode)
	{
		Parent = InParentNode;
		return this;
	}

	// Children/InlineChildren:
	Self* AddChild(const TSharedPtr<Self>& ChildNode, const bool bIsInline = false)
	{
		ensure(!ChildNode->IsRoot());
		ensure(!IsSeparator());

		ChildNode->SetParent(this->AsShared());
		if (bIsInline)
		{
			InlineChildren.Add(ChildNode);
		}
		else
		{
			Children.Add(ChildNode);
		}
		return this;
	}
	bool HasInlineChildren() const { return InlineChildren.Num() > 0; }
	bool HasChildren() const { return Children.Num() > 0; }

	const TArray<TSharedPtr<Self>>& GetChildren() const { return Children; }
	const TArray<TSharedPtr<Self>>& GetInlineChildren() const { return InlineChildren; }
	void GetVisibleChildren(TArray<TSharedPtr<Self>>& OutChildren)
	{
		for (const TSharedPtr<Self>& Child : Children)
		{
			if (Child->IsVisible())
			{
				OutChildren.Add(Child);
			}
		}
	}
	Self* SetChildren(const TArray<TSharedPtr<Self>>& InChildren)
	{
		Children = InChildren;
		for (const TSharedPtr<Self>& Child : Children)
		{
			Child->SetParent(this->AsShared());
		}
		return this;
	}
	Self* SetInlineChildren(const TArray<TSharedPtr<Self>>& InChildren)
	{
		InlineChildren = InChildren;
		for (const TSharedPtr<Self>& Child : InlineChildren)
		{
			Child->SetParent(this->AsShared());
		}
		return this;
	}
	void ClearChildren()
	{
		Children.Empty();
		InlineChildren.Empty();
	}

	/** Is this the root node? Aka no parent. */
	bool IsRoot() const { return !Parent.IsValid();}

	/** Is this the leaf node? Aka no children. */
	bool IsLeaf() const { return Children.Num() == 0; }

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
							TextType == EDialogueTreeNodeTextType::BoolVariableDialogue);
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
							TextType == EDialogueTreeNodeTextType::BoolVariableGraphNode);
	}
	bool IsEdgeNodeText()
	{
		return IsText() && (TextType == EDialogueTreeNodeTextType::ConditionEdgeNode ||
							TextType == EDialogueTreeNodeTextType::IntVariableEdgeNode ||
							TextType == EDialogueTreeNodeTextType::FloatVariableEdgeNode ||
							TextType == EDialogueTreeNodeTextType::BoolVariableEdgeNode);
	}

	/**
	 * Recursively collects all child/grandchild/decendent nodes
	 * Aka Flattened tree.
	 * @param  OutNodeArray	The array to fill out with decendent nodes.
	 */
	void GetAllNodes(TArray<TSharedPtr<Self>>& OutNodeArray) const;

	/**
	 * Takes the tree view and expands its elements for each child.
	 *
	 * @param  TreeView		The tree responsible for visualizing this node hierarchy.
	 * @param  bRecursive	Determines if you want children/descendants to expand their children as well.
	 */
	void ExpandAllChildren(TSharedPtr<STreeView<TSharedPtr<Self>>> TreeView, bool bRecursive = true);

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
		return 	Text == Other.GetText() &&
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
	// The Displayed Text
	FName Text;

	// Used to store Event, Condition, IntName, Dialogue name etc
	FName VariableName = NAME_None;

	/** The type of this item. */
	EDialogueTreeNodeType Type = EDialogueTreeNodeType::Text;

	// Specific category type, only used if Type is Category.
	EDialogueTreeNodeCategoryType CategoryType = EDialogueTreeNodeCategoryType::Default;

	// Specific text type, only used if the Type is Text.
	EDialogueTreeNodeTextType TextType = EDialogueTreeNodeTextType::Default;

	// Is this node displayed?
	bool bIsVisible = true;

	/** The node that this is a direct child of (empty if this is a root node) */
	TWeakPtr<Self> Parent;

	// Inline Nodes, Nodes that are displayed in the same line as this Node
	TArray<TSharedPtr<Self>> InlineChildren;

	// Children of this item
	TArray<TSharedPtr<Self>> Children;
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
	FDialogueBrowserTreeCategoryNode(const FName& InText, const EDialogueTreeNodeCategoryType InCategoryType,
									FDialogueBrowserTreeNodePtr InParent);
};


/** Node results that represents the Participant Name. */
class FDialogueBrowserTreeParticipantNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeParticipantNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeParticipantNode(const FName& InText, FDialogueBrowserTreeNodePtr InParent,
										const FName& InParticipantName);

	// ParticipantName:
	const FName GetParentParticipantName() const override;
	Self* SetParticipantName(const FName& InParticipantName)
	{
		ParticipantName = InParticipantName;
		return this;
	}

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
	FDialogueBrowserTreeCategoryParticipantNode(const FName& InText, FDialogueBrowserTreeNodePtr InParent,
		const FName& InParticipantName);
};


/** Node results that represents the Dialogue. */
class FDialogueBrowserTreeDialogueNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeDialogueNode(const FName& InText, FDialogueBrowserTreeNodePtr InParent,
									const TWeakObjectPtr<const UDlgDialogue>& InObject);

	// Dialogue:
	const TWeakObjectPtr<const UDlgDialogue>& GetDialogue() const { return Dialogue; }

	// FDialogueBrowserTreeNode Interface
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
	FDialogueBrowserTreeGraphNode(const FName& InText, FDialogueBrowserTreeNodePtr InParent,
								const TWeakObjectPtr<const UDialogueGraphNode>& InObject);

	// GraphNode:
	const TWeakObjectPtr<const UDialogueGraphNode>& GetGraphNode() const { return GraphNode; }

	// FDialogueBrowserTreeNode Interface
	FReply OnClick() override;

protected:
	/** The GraphNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode> GraphNode;
};


/** Node results that represents the EdgeNode. */
class FDialogueBrowserTreeEdgeNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;\
public:
	FDialogueBrowserTreeEdgeNode(const FName& InText, FDialogueBrowserTreeNodePtr InParent,
								const TWeakObjectPtr<const UDialogueGraphNode_Edge>& InObject);

	// EdgeNode:
	const TWeakObjectPtr<const UDialogueGraphNode_Edge>& GetEdgeNode() const { return EdgeNode; }

	// FDialogueBrowserTreeNode Interface
	FReply OnClick() override;

protected:
	/** The EdgeNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};
