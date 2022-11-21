// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "DlgSystem/TreeViewHelpers/DlgTreeViewNode.h"

// Type of Variable
enum class EDlgDataDisplayVariableTreeNodeType : uint8
{
	Default = 0,

	Event,
	Condition,

	Integer,
	Float,
	Bool,
	FName,

	ClassInteger,
	ClassFloat,
	ClassBool,
	ClassFName,
	ClassFText
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

/* Base class node for all Nodes in the DlgDataDisplayWindow  */
class DLGSYSTEM_API FDlgDataDisplayTreeNode : public FDlgTreeViewNode<FDlgDataDisplayTreeNode>
{
	typedef FDlgDataDisplayTreeNode Self;
	typedef FDlgTreeViewNode Super;
public:
	FDlgDataDisplayTreeNode(const FText& InDisplayText, const TSharedPtr<Self>& InParent);

	// Categories
	EDlgDataDisplayTextTreeNodeType GetTextType() const { return TextType; }
	EDlgDataDisplayCategoryTreeNodeType GetCategoryType() const { return CategoryType; }

	/** Gets the Actor that has this Node belongs to. */
	virtual TWeakObjectPtr<AActor> GetParentActor() const;

	virtual bool IsText() const { return TextType != EDlgDataDisplayTextTreeNodeType::Default; }
	virtual bool IsCategory() const { return false; }
	virtual bool IsSeparator() const { return false; }

	/** Is this equal with Other? */
	virtual bool IsEqual(const Self& Other)
	{
		return TextType == Other.GetTextType() &&
			CategoryType == Other.GetCategoryType() &&
			DisplayText.EqualTo(Other.GetDisplayText()) &&
			GetParentActor() == Other.GetParentActor();
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
	EDlgDataDisplayCategoryTreeNodeType CategoryType;

	// Specific text type, only used if the Type is Text.
	EDlgDataDisplayTextTreeNodeType TextType;
};


/** Root node of the Dialogue Data Display */
class DLGSYSTEM_API FDlgDataDisplayTreeRootNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeRootNode();
};


/** Node result that Represents the Actor. */
class DLGSYSTEM_API FDlgDataDisplayTreeActorNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeActorNode(
		const FText& InDisplayText,
		const TSharedPtr<FDlgDataDisplayTreeNode>& InParent,
		TWeakObjectPtr<AActor> InActor
	);

	/** FDlgDataDisplayTreeNode interface */
	TWeakObjectPtr<AActor> GetParentActor() const override;
	/** End FDlgDataDisplayTreeNode interface */

protected:
	/** The Actor this represents. */
	TWeakObjectPtr<AActor> Actor;
};


/** Node Representing a Category. */
class DLGSYSTEM_API FDlgDataDisplayTreeCategoryNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeCategoryNode(
		const FText& InDisplayText,
		const TSharedPtr<FDlgDataDisplayTreeNode>& InParent,
		EDlgDataDisplayCategoryTreeNodeType InCategoryType
	);

	bool IsText() const override { return false; }
	bool IsCategory() const  override { return CategoryType != EDlgDataDisplayCategoryTreeNodeType::Default; }
};


/** Node result that Represents the Variable (Int/Float/Condition). */
class DLGSYSTEM_API FDlgDataDisplayTreeVariableNode : public FDlgDataDisplayTreeNode
{
	typedef FDlgDataDisplayTreeVariableNode Self;
	typedef FDlgDataDisplayTreeNode Super;
public:
	FDlgDataDisplayTreeVariableNode(
		const FText& InDisplayText,
		const TSharedPtr<FDlgDataDisplayTreeNode>& InParent,
		FName InVariableName,
		EDlgDataDisplayVariableTreeNodeType InVariableType
	);

	// VariableName:
	FName GetVariableName() const { return VariableName; }

	// VariableValue:
	void SetVariableValue(const FString& InVariableValue) { VariableValue = InVariableValue; }
	FString GetVariableValue() const { return VariableValue; }

	// VariableType:
	EDlgDataDisplayVariableTreeNodeType GetVariableType() const { return VariableType; }

	bool IsEqual(const Super& Other) override
	{
		if (const Self* OtherSelf = static_cast<const Self*>(&Other))
		{
			return VariableName == OtherSelf->GetVariableName() &&
				VariableType == OtherSelf->GetVariableType() &&
				VariableValue == OtherSelf->GetVariableValue() &&
				Super::IsEqual(Other);
		}
		return false;
	}

protected:
	/** Used to store the name Event, Condition, IntName, etc */
	FName VariableName = NAME_None;

	/** The Value of the Variable. Not Used for variable types that do not have a value (like event). */
	FString VariableValue;

	/** What type is this Variable? */
	EDlgDataDisplayVariableTreeNodeType VariableType;
};
