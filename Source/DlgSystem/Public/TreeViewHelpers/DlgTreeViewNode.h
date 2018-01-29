// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "STreeView.h"

/** Base Tree Node for all Nodes used in the STreeView. */
template <class SelfType>
class FDlgTreeViewNode : public TSharedFromThis<SelfType>
{
public:
	/* Create a listing for a search result*/
	FDlgTreeViewNode(const FText& InDisplayText, TSharedPtr<SelfType> InParent)
		: Parent(InParent), DisplayText(InDisplayText) {}
	virtual ~FDlgTreeViewNode() {}

	/* Called when user clicks on the search item */
	virtual FReply OnClick()
	{
		// If there is a parent, handle it using the parent's functionality
		if (Parent.IsValid())
		{
			return Parent.Pin()->OnClick();
		}

		return FReply::Unhandled();
	}

	// DisplayText:
	FText GetDisplayText() const { return DisplayText; }
	FName GetDisplayTextAsFName() const { return FName(*DisplayText.ToString()); }
	void SetDisplayText(const FText& InText) { DisplayText = InText; }
	bool DoesDisplayTextContains(const FString& InSearch, ESearchCase::Type SearchCase = ESearchCase::IgnoreCase)
	{
		return DisplayText.ToString().Contains(InSearch, SearchCase);
	}

	// Parent:
	bool HasParent() const { return Parent.IsValid(); }
	TWeakPtr<SelfType> GetParent() const { return Parent; }
	void SetParent(TWeakPtr<SelfType> InParentNode) { Parent = InParentNode; }
	void ClearParent() { Parent.Reset(); }

	// Children:
	bool HasChildren() const { return Children.Num() > 0; }
	const TArray<TSharedPtr<SelfType>>& GetChildren() const { return Children; }
	void GetVisibleChildren(TArray<TSharedPtr<SelfType>>& OutChildren)
	{
		for (const TSharedPtr<SelfType>& Child : Children)
		{
			if (Child->IsVisible())
			{
				OutChildren.Add(Child);
			}
		}
	}
	virtual void AddChild(const TSharedPtr<SelfType>& ChildNode)
	{
		ensure(!ChildNode->IsRoot());
		ChildNode->SetParent(this->AsShared());
		Children.Add(ChildNode);
	}
	virtual void SetChildren(const TArray<TSharedPtr<SelfType>>& InChildren)
	{
		Children = InChildren;
		for (const TSharedPtr<SelfType>& Child : Children)
		{
			ensure(!Child->IsRoot());
			Child->SetParent(this->AsShared());
		}
	}
	virtual void ClearChildren()
	{
		Children.Empty();
	}

	// bIsVisible:
	bool IsVisible() const { return bIsVisible; }
	void SetIsVisible(const bool InIsVisible) { bIsVisible = InIsVisible; }

	/** Is this the root node? Aka no parent. */
	bool IsRoot() const { return !Parent.IsValid(); }

	/** Is this the leaf node? Aka no children. */
	bool IsLeaf() const { return Children.Num() == 0; }

	/**
	 * Takes the tree view and expands its elements for each child.
	 *
	 * @param  TreeView		The tree responsible for visualizing this node hierarchy.
	 * @param  bRecursive	Determines if you want children/descendants to expand their children as well.
	 */
	void ExpandAllChildren(TSharedPtr<STreeView<TSharedPtr<SelfType>>> TreeView, bool bRecursive = true)
	{
		static constexpr bool bShouldExpandItem = true;
		if (!HasChildren())
		{
			return;
		}

		TreeView->SetItemExpansion(this->AsShared(), bShouldExpandItem);
		for (TSharedPtr<SelfType>& ChildNode : Children)
		{
			if (bRecursive)
			{
				// recursive on all children.
				ChildNode->ExpandAllChildren(TreeView, bRecursive);
			}
			else
			{
				// Only direct children
				TreeView->SetItemExpansion(ChildNode, bShouldExpandItem);
			}
		}
	}

	/**
	 * Recursively collects all child/grandchild/decendent nodes.
	 * Aka Flattened tree.
	 * @param  OutNodeArray	The array to fill out with decendent nodes.
	 */
	void GetAllNodes(TArray<TSharedPtr<SelfType>>& OutNodeArray) const
	{
		for (const TSharedPtr<SelfType>& ChildNode : Children)
		{
			OutNodeArray.Add(ChildNode);
			ChildNode->GetAllNodes(OutNodeArray);
		}
	}

protected:
	/** Any children listed under this node. */
	TArray<TSharedPtr<SelfType>> Children;

	/** The node that this is a direct child of (empty if this is a root node)  */
	TWeakPtr<SelfType> Parent;

	/** The displayed text for this item. */
	FText DisplayText;

	/** Is this node displayed? */
	bool bIsVisible = true;
};
