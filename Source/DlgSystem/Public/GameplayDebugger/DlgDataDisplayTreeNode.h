// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Reply.h"
#include "UObject/WeakObjectPtr.h"
#include "Containers/Set.h"
#include "GameFramework/Actor.h"

#include "TreeViewHelpers/DlgTreeViewNode.h"

// Type of Variable
enum class EDlgDataDisplayVariableTreeNodeType : uint8
{
	Default = 0,

	Event,
	Condition,
	Integer,
	Float,
	Bool,
	FName
};

// Type of Text
enum class EDlgDataDisplayTextTreeNodeType : uint8
{
	Default = 0,

	Actor,
	Variable
};

// Type of Category
enum class EDlgDataDisplayCategoryTreeNodeType : uint8
{
	Default = 0,

	Event,
	Condition,
	Variables
};

class FDlgDataDisplayTreeNode;
typedef TSharedPtr<FDlgDataDisplayTreeNode> FDlgDataDisplayTreeNodePtr;

/* Base class node for all Nodes in the DlgDataDisplayWindow  */
class FDlgDataDisplayTreeNode : public FDlgTreeViewNode<FDlgDataDisplayTreeNode>
{
	typedef FDlgDataDisplayTreeNode Self;
	typedef FDlgTreeViewNode Super;
public:
	FDlgDataDisplayTreeNode(const FText& InDisplayText, TSharedPtr<Self> InParent);

	// Categories
	EDlgDataDisplayTextTreeNodeType GetTextType() const { return TextType; }
	EDlgDataDisplayCategoryTreeNodeType GetCategoryType() const { return CategoryType; }

	/** Gets the Actor that has this Node belongs to. */
	virtual TWeakObjectPtr<AActor> GetParentActor() const;

	virtual bool IsText() const { return TextType != EDlgDataDisplayTextTreeNodeType::Default; }
	virtual bool IsCategory() const { return false; }
	virtual bool IsSeparator() const { return false; }

protected:
	// Specific category type, only used if Type is Category.
	EDlgDataDisplayCategoryTreeNodeType CategoryType;

	// Specific text type, only used if the Type is Text.
	EDlgDataDisplayTextTreeNodeType TextType;
};


/** Root node of the Dialogue Data Display */
class FDlgDataDisplayTreeRootNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeRootNode();
};


/** Node result that Represents the Actor. */
class FDlgDataDisplayTreeActorNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeActorNode(const FText& InDisplayText, FDlgDataDisplayTreeNodePtr InParent,
		TWeakObjectPtr<AActor> InActor);

	/** FDlgDataDisplayTreeNode interface */
	TWeakObjectPtr<AActor> GetParentActor() const override;
	/** End FDlgDataDisplayTreeNode interface */

protected:
	/** The Actor this represents. */
	TWeakObjectPtr<AActor> Actor;
};


/** Node Representing a Category. */
class FDlgDataDisplayTreeCategoryNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeCategoryNode(const FText& InDisplayText, FDlgDataDisplayTreeNodePtr InParent,
		const EDlgDataDisplayCategoryTreeNodeType InCategoryType);

	bool IsText() const override { return false; }
	bool IsCategory() const  override { return CategoryType != EDlgDataDisplayCategoryTreeNodeType::Default; }
};


/** Node result that Represents the Variable (Int/Float/Condition). */
class FDlgDataDisplayTreeVariableNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeVariableNode(const FText& InDisplayText, FDlgDataDisplayTreeNodePtr InParent,
		const FName& InVariableName, const EDlgDataDisplayVariableTreeNodeType InVariableType);

	// VariableName:
	FName GetVariableName() const { return VariableName; }

	// VariableValue:
	void SetVariableValue(const FString& InVariableValue) { VariableValue = InVariableValue; }
	FString GetVariableValue() const { return VariableValue; }

	// VariableType:
	EDlgDataDisplayVariableTreeNodeType GetVariableType() const { return VariableType; }

protected:
	/** Used to store the name Event, Condition, IntName, etc */
	FName VariableName = NAME_None;

	/** The Value of the Variable. Not Used for variable types that do not have a value (like event). */
	FString VariableValue;

	/** What type is this Variable? */
	EDlgDataDisplayVariableTreeNodeType VariableType;
};
