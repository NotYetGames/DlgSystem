// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STreeView.h"

#include "TreeViewHelpers/DlgTreeViewNode.h"

class FDialogueBrowserTreeNode;
class UDlgDialogue;
class UDialogueGraphNode;
class UDialogueGraphNode_Edge;

// The types of categories.
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

	ClassVariable,
	ClassVariableInt,
	ClassVariableFloat,
	ClassVariableBool,
	ClassVariableFName,
	ClassVariableFText,

	Max
};

enum class EDialogueTreeNodeTextType : uint8
{
	Default = 0,

	ParticipantDialogue,
	ParticipantEvent,
	ParticipantCustomEvent,
	ParticipantCondition,
	ParticipantVariableInt,
	ParticipantVariableFloat,
	ParticipantVariableBool,
	ParticipantVariableFName,
	ParticipantClassVariableInt,
	ParticipantClassVariableFloat,
	ParticipantClassVariableBool,
	ParticipantClassVariableFName,
	ParticipantClassVariableFText,

	EventDialogue,
	CustomEventDialogue,
	EventGraphNode,
	CustomEventGraphNode,

	ConditionDialogue,
	ConditionGraphNode,
	ConditionEdgeNode,

	IntVariableDialogue,
	IntClassVariableDialogue,
	IntVariableGraphNode,
	IntVariableEdgeNode,

	FloatVariableDialogue,
	FloatClassVariableDialogue,
	FloatVariableGraphNode,
	FloatVariableEdgeNode,

	BoolVariableDialogue,
	BoolClassVariableDialogue,
	BoolVariableGraphNode,
	BoolVariableEdgeNode,

	FNameVariableDialogue,
	FNameClassVariableDialogue,
	FNameVariableGraphNode,
	FNameVariableEdgeNode,

	FTextClassVariableDialogue,
	FTextVariableGraphNode,
	FTextVariableEdgeNode,

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
	FDialogueBrowserTreeNode(const FText& InDisplayText, const TSharedPtr<Self>& InParent);

	/** Gets the Participant Name that this Node belongs to. This must always return a valid value. */
	virtual FName GetParentParticipantName() const;

	/** Gets the Variable name that this Node belongs to if any. This could be empty in most cases. */
	virtual FName GetParentVariableName() const;

	/** Gets the Class that this Node belongs to if any. This could be empty in most cases. */
	virtual UClass* GetParentClass() const;

	//
	// Getters for the properties
	//

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

	void AddInlineChild(const TSharedPtr<Self>& ChildNode, bool bIsInline = false)
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

	// Checks type of this Node.
	virtual bool IsText() const { return TextType != EDialogueTreeNodeTextType::Default; }
	virtual bool IsCategory() const { return false; }
	virtual bool IsSeparator() const { return false; }
	bool IsDialogueText() const
	{
		return IsText() &&
			  (TextType == EDialogueTreeNodeTextType::ParticipantDialogue
			|| TextType == EDialogueTreeNodeTextType::EventDialogue
			|| TextType == EDialogueTreeNodeTextType::CustomEventDialogue
			|| TextType == EDialogueTreeNodeTextType::ConditionDialogue
			|| TextType == EDialogueTreeNodeTextType::IntVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::FloatVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::BoolVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::FNameVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::IntClassVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::FloatClassVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::BoolClassVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::FTextClassVariableDialogue
			|| TextType == EDialogueTreeNodeTextType::FNameClassVariableDialogue);
	}
	bool IsEventText() const
	{
		return IsText() && TextType == EDialogueTreeNodeTextType::ParticipantEvent;
	}
	bool IsCustomEventText() const
	{
		return IsText() && TextType == EDialogueTreeNodeTextType::ParticipantCustomEvent;
	}
	bool IsConditionText() const
	{
		return IsText() && (TextType == EDialogueTreeNodeTextType::ParticipantCondition);
	}
	bool IsGraphNodeText() const
	{
		return IsText() &&
			  (TextType == EDialogueTreeNodeTextType::EventGraphNode
			|| TextType == EDialogueTreeNodeTextType::CustomEventGraphNode
			|| TextType == EDialogueTreeNodeTextType::ConditionGraphNode
			|| TextType == EDialogueTreeNodeTextType::IntVariableGraphNode
			|| TextType == EDialogueTreeNodeTextType::FloatVariableGraphNode
			|| TextType == EDialogueTreeNodeTextType::BoolVariableGraphNode
			|| TextType == EDialogueTreeNodeTextType::FTextVariableGraphNode
			|| TextType == EDialogueTreeNodeTextType::FNameVariableGraphNode);
	}
	bool IsEdgeNodeText()
	{
		return IsText() &&
			  (TextType == EDialogueTreeNodeTextType::ConditionEdgeNode
			|| TextType == EDialogueTreeNodeTextType::IntVariableEdgeNode
			|| TextType == EDialogueTreeNodeTextType::FloatVariableEdgeNode
			|| TextType == EDialogueTreeNodeTextType::BoolVariableEdgeNode
			|| TextType == EDialogueTreeNodeTextType::FTextVariableEdgeNode
			|| TextType == EDialogueTreeNodeTextType::FNameVariableEdgeNode);
	}

	// Gets the textual representation of this item
	FString ToString() const;

	// Is this equal with Other?
	virtual bool IsEqual(const Self& Other)
	{
		return TextType == Other.GetTextType() &&
			CategoryType == Other.GetCategoryType() &&
			DisplayText.EqualTo(Other.GetDisplayText()) &&
			GetParentParticipantName() == Other.GetParentParticipantName() &&
			GetParentVariableName() == Other.GetParentVariableName();
	}

	bool operator==(const Self& Other)
	{
		return IsEqual(Other);
	}

protected:
	// FDlgTreeViewNode Interface
	void PostFilterPathsToNodes(const TSharedPtr<Self>& Child) override
	{
		Super::PostFilterPathsToNodes(Child);

		// Hide separators
		if (Child->IsSeparator())
		{
			Child->SetIsVisible(false);
		}
		// Some child has the InSearch or this Node has the text
		//Children[Index]->SetIsVisible(NumBefore != OutNodes.Num() || Children[Index]->TextContains(InSearch));
	}

	void PostBuildPathToTopMostParent(const TSharedPtr<Self>& CurrentParentNode) override
	{
		Super::PostBuildPathToTopMostParent(CurrentParentNode);
		check(!CurrentParentNode->IsSeparator());
	}

	bool FilterIsChildVisible(const TSharedPtr<Self>& GrandChild) override
	{
		return !GrandChild->IsSeparator() && !GrandChild->IsCategory() && Super::FilterIsChildVisible(GrandChild);
	}

	bool FilterDoesChildContainText(const TSharedPtr<Self>& Child, const FString& InSearch) override
	{
		return !Child->IsSeparator() && Super::FilterDoesChildContainText(Child, InSearch);
	}

protected:
	// Specific category type, only used if Type is Category.
	EDialogueTreeNodeCategoryType CategoryType = EDialogueTreeNodeCategoryType::Default;

	// Specific text type, only used if the Type is Text.
	EDialogueTreeNodeTextType TextType = EDialogueTreeNodeTextType::Default;

	// Inline Nodes, Nodes that are displayed in the same line as this Node
	TArray<TSharedPtr<Self>> InlineChildren;
};


// Root node of the Dialogue browser
class FDialogueBrowserTreeRootNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeRootNode();
};


// Separator node of the Dialogue browser
class FDialogueBrowserTreeSeparatorNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeSeparatorNode(const TSharedPtr<FDialogueBrowserTreeNode>& InParent = nullptr);
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
	FDialogueBrowserTreeCategoryNode(
		const FText& InDisplayText,
		const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
		EDialogueTreeNodeCategoryType InCategoryType
	);

	bool IsText() const override { return false; }
	bool IsCategory() const  override { return CategoryType != EDialogueTreeNodeCategoryType::Default; }
};


// Node results that represents the Participant Name.
class FDialogueBrowserTreeParticipantNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeParticipantNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeParticipantNode(
		const FText& InDisplayText,
		const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
		FName InParticipantName
	);

	// ParticipantName:
	FName GetParentParticipantName() const override;

protected:
	/** The Participant Name it represents. */
	FName ParticipantName = NAME_None;
};


// Node results that represents a Variable Name.
class FDialogueBrowserTreeVariableNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeVariableNode Self;
	typedef FDialogueBrowserTreeNode Super;

public:
	FDialogueBrowserTreeVariableNode(
		const FText& InDisplayText,
		const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
		FName InVariableName
	);

	// VariableName:
	FName GetParentVariableName() const override;

protected:
	// Used to store Event, Condition, IntName, Dialogue name etc
	FName VariableName = NAME_None;
};

// Node result that represents a custom object
class FDialogueBrowserTreeCustomObjectNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeCustomObjectNode Self;
	typedef FDialogueBrowserTreeNode Super;

public:
	FDialogueBrowserTreeCustomObjectNode(
        const FText& InDisplayText,
        const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
        UClass* ObjectClass
    );

	// Class
	UClass* GetClass() const { return Class.Get(); }
	UClass* GetParentClass() const { return GetClass(); }

protected:
	// Class this represents
	TWeakObjectPtr<UClass> Class = nullptr;
};


// Similar to the FDialogueBrowserTreeParticipantNode only this is a Category
class FDialogueBrowserTreeCategoryParticipantNode : public FDialogueBrowserTreeParticipantNode
{
	typedef FDialogueBrowserTreeParticipantNode Super;
public:
	FDialogueBrowserTreeCategoryParticipantNode(
		const FText& InDisplayText,
		const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
		FName InParticipantName
	);

	bool IsText() const override { return false; }
	bool IsCategory() const override { return true; }
};


// Node results that represents the Dialogue.
class FDialogueBrowserTreeDialogueNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeDialogueNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeDialogueNode(
		const FText& InDisplayText,
		const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
		const TWeakObjectPtr<const UDlgDialogue>& InObject
	);

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
	// The Dialogue this represents.
	TWeakObjectPtr<const UDlgDialogue> Dialogue;
};


// Node results that represents the GraphNode.
class FDialogueBrowserTreeGraphNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeGraphNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeGraphNode(
		const FText& InDisplayText,
		const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
		const TWeakObjectPtr<const UDialogueGraphNode>& InObject
	);

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
	// The GraphNode this represents.
	TWeakObjectPtr<const UDialogueGraphNode> GraphNode;
};


// Node results that represents the EdgeNode.
class FDialogueBrowserTreeEdgeNode : public FDialogueBrowserTreeNode
{
	typedef FDialogueBrowserTreeEdgeNode Self;
	typedef FDialogueBrowserTreeNode Super;
public:
	FDialogueBrowserTreeEdgeNode(
		const FText& InDisplayText,
		const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
		const TWeakObjectPtr<const UDialogueGraphNode_Edge>& InObject
	);

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
	// The EdgeNode this represents.
	TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode;
};
