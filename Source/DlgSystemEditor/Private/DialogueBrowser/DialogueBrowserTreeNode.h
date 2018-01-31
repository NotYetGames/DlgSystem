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


/**
 * Defines the singular Text item in the Tree.
 * For Category type see FDialogueBrowserTreeCategoryNode.
 */
class FDialogueBrowserTreeNode : public FDlgTreeViewNode<FDialogueBrowserTreeNode>
{
	typedef FDialogueBrowserTreeNode Self;
	typedef FDlgTreeViewNode Super;

public:
	FDialogueBrowserTreeNode(const FText& InDisplayText, TSharedPtr<Self> InParent);

	/** Gets the Participant Name that this Node belongs to. This must always return a valid value. */
	virtual const FName GetParentParticipantName() const;

	/** Gets the Variable name that this Node belongs to if any. This could be empty in most cases. */
	virtual const FName GetParentVariableName() const;

	/** Getters for the properties */

	// TextType:
	EDialogueTreeNodeTextType GetTextType() const { return TextType; }
	void SetTextType(EDialogueTreeNodeTextType InTextType) { TextType = InTextType; }

	// CategoryType:
	EDialogueTreeNodeCategoryType GetCategoryType() const { return CategoryType; }

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
	virtual bool IsText() const { return TextType != EDialogueTreeNodeTextType::Default; }
	virtual bool IsCategory() const { return false; }
	virtual bool IsSeparator() const { return false; }
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
		return 	GetParentParticipantName() == Other.GetParentParticipantName() &&
				GetParentVariableName() == Other.GetParentVariableName() &&
				DisplayText.EqualTo(Other.GetDisplayText()) &&
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
	bool IsText() const override { return false; }
	bool IsCategory() const  override { return false; }
	bool IsSeparator() const override { return true; }
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

	bool IsText() const override { return false; }
	bool IsCategory() const  override { return CategoryType != EDialogueTreeNodeCategoryType::Default; }
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

protected:
	/** The Participant Name it represents. */
	FName ParticipantName = NAME_None;
};


/** Node results that represents a Variable Name. */
class FDialogueBrowserTreeVariableNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeVariableNode Self;
	typedef FDialogueBrowserTreeNode Super;

public:
	FDialogueBrowserTreeVariableNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
									const FName& InVariableName);

	// VariableName:
	const FName GetParentVariableName() const override;

protected:
	// Used to store Event, Condition, IntName, Dialogue name etc
	FName VariableName = NAME_None;
};


/** Similar to the FDialogueBrowserTreeParticipantNode only this is a Category */
class FDialogueBrowserTreeCategoryParticipantNode : public FDialogueBrowserTreeParticipantNode
{
	typedef FDialogueBrowserTreeParticipantNode Super;
public:
	FDialogueBrowserTreeCategoryParticipantNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
		const FName& InParticipantName);

	bool IsText() const override { return false; }
	bool IsCategory() const override { return true; }
};


/** Node results that represents the Dialogue. */
class FDialogueBrowserTreeDialogueNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeDialogueNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeDialogueNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
									const TWeakObjectPtr<const UDlgDialogue>& InObject);

	// Dialogue:
	const TWeakObjectPtr<const UDlgDialogue>& GetDialogue() const { return Dialogue; }
	FReply OnClick() override;

	bool IsEqual(const Super& Other) override
	{
		if (const Self* OtherSelf = static_cast<const Self*>(&Other))
		{
			return Dialogue == OtherSelf->GetDialogue() && Super::IsEqual(Other);
		}
		return false;
	}

protected:
	/** The Dialogue this represents. */
	TWeakObjectPtr<const UDlgDialogue> Dialogue;
};


/** Node results that represents the GraphNode. */
class FDialogueBrowserTreeGraphNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeGraphNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeGraphNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
								const TWeakObjectPtr<const UDialogueGraphNode>& InObject);

	// GraphNode:
	const TWeakObjectPtr<const UDialogueGraphNode>& GetGraphNode() const { return GraphNode; }
	FReply OnClick() override;

	bool IsEqual(const Super& Other) override
	{
		if (const Self* OtherSelf = static_cast<const Self*>(&Other))
		{
			return GraphNode == OtherSelf->GetGraphNode() && Super::IsEqual(Other);
		}
		return false;
	}

protected:
	/** The GraphNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode> GraphNode;
};


/** Node results that represents the EdgeNode. */
class FDialogueBrowserTreeEdgeNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeEdgeNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeEdgeNode(const FText& InDisplayText, FDialogueBrowserTreeNodePtr InParent,
								const TWeakObjectPtr<const UDialogueGraphNode_Edge>& InObject);

	// EdgeNode:
	const TWeakObjectPtr<const UDialogueGraphNode_Edge>& GetEdgeNode() const { return EdgeNode; }
	FReply OnClick() override;

	bool IsEqual(const Super& Other) override
	{
		if (const Self* OtherSelf = static_cast<const Self*>(&Other))
		{
			return EdgeNode == OtherSelf->GetEdgeNode() && Super::IsEqual(Other);
		}
		return false;
	}

protected:
	/** The EdgeNode this represents. */
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};
