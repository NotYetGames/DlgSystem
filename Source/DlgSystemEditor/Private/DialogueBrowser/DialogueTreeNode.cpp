// Fill out your copyright notice in the Description page of Project Settings.
#include "DialogueTreeNode.h"

#include "STreeView.h"

void FDialogueTreeNode::GetAllNodes(TArray<FDialogueTreeNodePtr>& OutNodeArray) const
{
	for (const FDialogueTreeNodePtr& ChildNode : Children)
	{
		OutNodeArray.Add(ChildNode);
		ChildNode->GetAllNodes(OutNodeArray);
	}
}

void FDialogueTreeNode::ExpandAllChildren(TSharedPtr<STreeView<FDialogueTreeNodePtr>> TreeView, bool bRecursive /*= true*/)
{
	if (!HasChildren())
	{
		return;
	}
	static constexpr bool bShouldExpandItem = true;

	TreeView->SetItemExpansion(this->AsShared(), bShouldExpandItem);
	for (FDialogueTreeNodePtr& ChildNode : Children)
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

void FDialogueTreeNode::GetPathToChildThatContainsText(const SelfPtr& Child, const FString& InSearch, TArray<TArray<SelfPtr>>& OutNodes)
{
	// Child has text, build path to it
	bool bChildIsVisible;
	if (!Child->IsSeparator() && Child->TextContains(InSearch))
	{
		bChildIsVisible = true;

		// Build path to top most parent
		SelfPtr CurrentNode = Child;
		TArray<SelfPtr> ChildOutNodes;
		while (CurrentNode->HasParentNode())
		{
			ChildOutNodes.Add(CurrentNode);

			// Parents are visible too
			SelfPtr CurrentParentNode = CurrentNode->GetParentNode().Pin();
			check(!CurrentParentNode->IsSeparator());
			CurrentParentNode->SetIsVisible(bChildIsVisible);

			// Advance up the tree
			CurrentNode = CurrentParentNode;
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
	for (const SelfPtr& ChildItem : Child->GetChildren())
	{
		//const int32 NumBefore = OutNodes.Num();
		GetPathToChildThatContainsText(ChildItem, InSearch, OutNodes);

		ChildItem->SetIsVisible(bChildIsVisible && !ChildItem->IsSeparator() &&
								!ChildItem->IsCategory() && ChildItem->IsLeafNode());
	}
}

void FDialogueTreeNode::FilterPathsToNodesThatContainText(const FString& InSearch, TArray<TArray<SelfPtr>>& OutNodes)
{
	for (int32 Index = 0, Num = Children.Num(); Index < Num; Index++)
	{
		Children[Index]->SetIsVisible(false);
		//const int32 NumBefore = OutNodes.Num();
		GetPathToChildThatContainsText(Children[Index], InSearch, OutNodes);

		// Hide separators
		if (Children[Index]->IsSeparator())
		{
			Children[Index]->SetIsVisible(false);
		}

		// Some child has the InSearch or this Node has the text
		//Children[Index]->SetIsVisible(NumBefore != OutNodes.Num() || Children[Index]->TextContains(InSearch));
	}
}


FString FDialogueTreeNode::ToString() const
{
	static const ANSICHAR* EDialogueTreeItemCategoryTypeStringMap[] = {
		"Default",
		"Participant",
		"Dialogue",
		"Event",
		"Condition",
		"Variable",
		"VariableInt",
		"VariableFloat",
		"VariableBool",
		"Max"
	};

	FString Output = "FTextItem { ";

	auto AddKeyValueField = [&Output](const FString& Key, const FString& Value, const bool bAddSeparator = true)
	{
		if (bAddSeparator)
		{
			Output += " | ";
		}
		Output += Key + " = `" + Value + "`";
	};
	auto AddFNameToOutput = [&Output, &AddKeyValueField](const FString& Name, const FName& Value)
	{
		if (!Value.IsNone() && Value.IsValid())
		{
			AddKeyValueField(Name, Value.ToString());
		}
	};

	AddKeyValueField("Text", Text.ToString(), false);
	AddFNameToOutput("ParticipantName", ParticipantName);
	AddFNameToOutput("VariableName", VariableName);

	if (Object.IsValid())
	{
		AddKeyValueField("Object Name", Object.Get()->GetName());
	}
	AddKeyValueField("Children Num", FString::FromInt(Children.Num()));

	return Output + " }";
}

