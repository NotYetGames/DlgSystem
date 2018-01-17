// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "STreeView.h"

struct FDialogueTreeNode;
class UDlgDialogue;

typedef TSharedPtr<FDialogueTreeNode> FDialogueTreeNodePtr;

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

	Participant,
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
	RootNode,
	Separator,
	Category,

	Max
};

/**
 * Defines the singular item in the Tree.
 */
struct FDialogueTreeNode : TSharedFromThis<FDialogueTreeNode>
{
private:
	typedef FDialogueTreeNode Self;
	typedef TSharedPtr<Self> SelfPtr;

public:
	explicit FDialogueTreeNode(const FName& InText)
		: Text(InText)
	{
	}

	FDialogueTreeNode(const FName& InText, const FName& InParticipantName)
		: Text(InText),
		ParticipantName(InParticipantName)
	{
	}

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

	// ParticipantName:
	bool HasParticipantName() const { return ParticipantName.IsValid() && !ParticipantName.IsNone(); }
	const FName& GetParticipantName() const { return ParticipantName; }
	Self* SetParticipantName(const FName& InParticipantName)
	{
		ParticipantName = InParticipantName;
		return this;
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
	Self* SetType(EDialogueTreeNodeType InType)
	{
		Type = InType;
		return this;
	}

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

	// Object:
	const TWeakObjectPtr<UObject>& GetObject() const { return Object; }
	Self* SetObject(const TWeakObjectPtr<UObject>& InObject)
	{
		Object = InObject;
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
	bool HasParentNode() const { return ParentNode.IsValid(); }
	TWeakPtr<Self> GetParentNode() const { return ParentNode; }
	Self* SetParentNode(TWeakPtr<Self> InParentNode)
	{
		ParentNode = InParentNode;
		return this;
	}

	// Children/InlineChildren:
	Self* AddChild(const SelfPtr& ChildNode, const bool bIsInline = false)
	{
		ensure(!ChildNode->IsRoot());
		ensure(!IsSeparator());

		ChildNode->SetParentNode(this->AsShared());
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
	bool IsLeafNode() const { return Children.Num() == 0; }
	const TArray<SelfPtr>& GetChildren() const { return Children; }
	const TArray<SelfPtr>& GetInlineChildren() const { return InlineChildren; }
	void GetVisibleChildren(TArray<SelfPtr>& OutChildren)
	{
		for (const SelfPtr& Child : Children)
		{
			if (Child->IsVisible())
			{
				OutChildren.Add(Child);
			}
		}
	}
	Self* SetChildren(const TArray<SelfPtr>& InChildren)
	{
		Children = InChildren;
		for (const SelfPtr& Child : Children)
		{
			Child->SetParentNode(this->AsShared());
		}
		return this;
	}
	Self* SetInlineChildren(const TArray<SelfPtr>& InChildren)
	{
		InlineChildren = InChildren;
		for (const SelfPtr& Child : InlineChildren)
		{
			Child->SetParentNode(this->AsShared());
		}
		return this;
	}
	void ClearChildren()
	{
		Children.Empty();
		InlineChildren.Empty();
	}

	/** Checks type of this Node. */
	bool IsText() const { return Type == EDialogueTreeNodeType::Text; }
	bool IsCategory() const { return Type == EDialogueTreeNodeType::Category; }
	bool IsSeparator() const { return Type == EDialogueTreeNodeType::Separator; }
	bool IsRoot() const { return Type == EDialogueTreeNodeType::RootNode; }
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
	void GetAllNodes(TArray<SelfPtr>& OutNodeArray) const;

	/**
	 * Takes the tree view and expands its elements for each child.
	 *
	 * @param  TreeView		The tree responsible for visualizing this node hierarchy.
	 * @param  bRecursive	Determines if you want children/descendants to expand their children as well.
	 */
	void ExpandAllChildren(TSharedPtr<STreeView<SelfPtr>> TreeView, bool bRecursive = true);

	/**
	 * Filters the node so that it will only containt paths to nodes that contains the specified string.
	 * @param OutNodes	Array of arrays, each array inside represents a node path that remains to the Node that contains the InSearch
	 * @param InSearch The string to search by
	 */
	void FilterPathsToNodesThatContainText(const FString& InSearch, TArray<TArray<SelfPtr>>& OutNodes);

	/** Gets the textual representation of this item */
	FString ToString() const;

	/** Helper methods. */
	static SelfPtr Make(const FName& InText) { return MakeShareable(new Self(InText)); }
	static SelfPtr Make(const FName& InText, const FName& ParticipantName)
	{
		return MakeShareable(new Self(InText, ParticipantName));
	}
	static SelfPtr MakeCategory(const FName& InText, const FName& ParticipantName)
	{
		FDialogueTreeNodePtr Ptr = Make(InText, ParticipantName);
		Ptr->SetType(EDialogueTreeNodeType::Category);
		return Ptr;
	}

	static SelfPtr MakeSeparator()
	{
		FDialogueTreeNodePtr Ptr = Make(TEXT("SEPARATOR"));
		Ptr->SetType(EDialogueTreeNodeType::Separator);
		return Ptr;
	}

	static SelfPtr MakeRoot()
	{
		FDialogueTreeNodePtr Ptr = Make(TEXT("ROOT"));
		Ptr->SetType(EDialogueTreeNodeType::RootNode);
		return Ptr;
	}

	static SelfPtr DeepCopy(const SelfPtr& Other)
	{
		SelfPtr Copy = MakeShareable(new Self(Other->GetText()));
		Copy->SetParticipantName(Other->GetParticipantName());
		Copy->SetVariableName(Other->GetVariableName());
		Copy->SetType(Other->GetType());
		Copy->SetCategoryType(Other->GetCategoryType());
		Copy->SetTextType(Other->GetTextType());
		Copy->SetObject(Other->GetObject());

		// Deep copy children
		for (const SelfPtr& ChildItem : Other->GetInlineChildren())
		{
			Copy->AddChild(DeepCopy(ChildItem), true);
		}
		for (const SelfPtr& ChildItem : Other->GetChildren())
		{
			Copy->AddChild(DeepCopy(ChildItem), false);
		}

		return Copy;
	}

	bool operator==(const Self& Other)
	{
		return Text == Other.GetText() &&
				ParticipantName == Other.GetParticipantName() &&
				VariableName == Other.GetVariableName() &&
				Type == Other.GetType() &&
				CategoryType == Other.GetCategoryType() &&
				TextType == Other.GetTextType();
	 }

private:
	void GetPathToChildThatContainsText(const SelfPtr& Child, const FString& InSearch, TArray<TArray<SelfPtr>>& OutNodes);

protected:
	FName Text;

	// Variables that may be set or not.
	FName ParticipantName = NAME_None;

	// Used to store Event, Condition, IntName, etc
	FName VariableName = NAME_None;

	/** The type of this item. */
	EDialogueTreeNodeType Type = EDialogueTreeNodeType::Text;

	// Specific category type, only used if Type is Category.
	EDialogueTreeNodeCategoryType CategoryType = EDialogueTreeNodeCategoryType::Default;

	// Specific text type, only used if the Type is Text.
	EDialogueTreeNodeTextType TextType = EDialogueTreeNodeTextType::Default;

	// Is this node displayed?
	bool bIsVisible = true;

	// The object it represents if any.
	TWeakObjectPtr<UObject> Object;

	/** The node that this is a direct child of (empty if this is a root node) */
	TWeakPtr<Self> ParentNode;

	// Inline Nodes, Nodes that are displayed in the same line as this Node
	TArray<SelfPtr> InlineChildren;

	// Children of this item
	TArray<SelfPtr> Children;
};
