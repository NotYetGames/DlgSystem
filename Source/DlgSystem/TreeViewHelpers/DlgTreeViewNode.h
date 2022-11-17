// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STreeView.h"

/** Base Tree Node for all Nodes used in the STreeView. */
template <class SelfType>
class FDlgTreeViewNode : public TSharedFromThis<SelfType>
{
public:
	/* Create a listing for a search result*/
	FDlgTreeViewNode(const FText& InDisplayText, const TSharedPtr<SelfType>& InParent)
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
	void SetIsVisible(bool InIsVisible) { bIsVisible = InIsVisible; }

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
	void ExpandAllChildren(const TSharedPtr<STreeView<TSharedPtr<SelfType>>>& TreeView, bool bRecursive = true)
	{
		static constexpr bool bShouldExpandItem = true;
		if (!HasChildren())
		{
			return;
		}

		TreeView->SetItemExpansion(this->AsShared(), bShouldExpandItem);
		for (const TSharedPtr<SelfType>& ChildNode : Children)
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
	 * Recursively collects all child/grandchild/descendant nodes.
	 * Aka Flattened tree.
	 * @param  OutNodeArray	The array to fill out with descendant nodes.
	 */
	void GetAllNodes(TArray<TSharedPtr<SelfType>>& OutNodeArray) const
	{
		for (const TSharedPtr<SelfType>& ChildNode : Children)
		{
			OutNodeArray.Add(ChildNode);
			ChildNode->GetAllNodes(OutNodeArray);
		}
	}

	/**
	 * Searches the node so that the OutNodes will only contains paths to nodes that contains the specified string.
	 * @param InSearch		The string to search by
	 * @param OutNodes		Array of arrays, each array inside represents a node path that points to the Node that contains the InSearch
	 */
	virtual void FilterPathsToNodesThatContainText(const FString& InSearch, TArray<TArray<TSharedPtr<SelfType>>>& OutNodes)
	{
		for (int32 Index = 0, Num = Children.Num(); Index < Num; Index++)
		{
			TSharedPtr<SelfType> CurrentChild = Children[Index];
			CurrentChild->SetIsVisible(false);
			GetPathToChildThatContainsText(Children[Index], InSearch, OutNodes);

			// Let child classes handle this themselves
			PostFilterPathsToNodes(CurrentChild);
		}
	}

protected:
	/** Called inside FilterPathsToNodesThatContainText after we got the path for the current Child.  */
	virtual void PostFilterPathsToNodes(const TSharedPtr<SelfType>& Child) {}

	/** Called inside GetPathToChildThatContainsText after we advanced one parent in the path */
	virtual void PostBuildPathToTopMostParent(const TSharedPtr<SelfType>& CurrentParentNode) {}

	/** Called inside GetPathToChildThatContainsText after we got the path of the GrandChild  */
	virtual bool FilterIsChildVisible(const TSharedPtr<SelfType>& GrandChild)
	{
		return GrandChild->IsLeaf();
	}

	/** Called inside GetPathToChildThatContainsText to determine if the child has the text  */
	virtual bool FilterDoesChildContainText(const TSharedPtr<SelfType>& Child, const FString& InSearch)
	{
		return Child->DoesDisplayTextContains(InSearch);
	}

	virtual void GetPathToChildThatContainsText(
		const TSharedPtr<SelfType>& Child,
		const FString& InSearch,
		TArray<TArray<TSharedPtr<SelfType>>>& OutNodes
	)
	{
		// Child has text, build path to it
		bool bChildIsVisible;
		if (FilterDoesChildContainText(Child, InSearch))
		{
			bChildIsVisible = true;

			// Build path to top most parent
			TSharedPtr<SelfType> CurrentNode = Child;
			TArray<TSharedPtr<SelfType>> ChildOutNodes;
			while (CurrentNode->HasParent())
			{
				ChildOutNodes.Add(CurrentNode);

				// Parents are visible too
				TSharedPtr<SelfType> CurrentParentNode = CurrentNode->GetParent().Pin();
				CurrentParentNode->SetIsVisible(bChildIsVisible);

				// Advance up the tree
				CurrentNode = CurrentParentNode;

				PostBuildPathToTopMostParent(CurrentParentNode);
			}

			// reverse
			Algo::Reverse(ChildOutNodes);
			OutNodes.Emplace(ChildOutNodes);
		}
		else
		{
			bChildIsVisible = false;
		}
		Child->SetIsVisible(bChildIsVisible);

		// Check children
		for (const TSharedPtr<SelfType>& GrandChild : Child->GetChildren())
		{
			GetPathToChildThatContainsText(GrandChild, InSearch, OutNodes);
			GrandChild->SetIsVisible(bChildIsVisible && FilterIsChildVisible(GrandChild));
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
