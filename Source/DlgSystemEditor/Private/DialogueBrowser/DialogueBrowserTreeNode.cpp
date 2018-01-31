// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueBrowserTreeNode.h"

#include "STreeView.h"

#include "DialogueEditor/DialogueEditorUtilities.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeNode
FDialogueBrowserTreeNode::FDialogueBrowserTreeNode(const FText& InDisplayText, TSharedPtr<Self> InParent)
	: Super(InDisplayText, InParent)
{
}

const FName FDialogueBrowserTreeNode::GetParentParticipantName() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentParticipantName();
	}

	return NAME_None;
}

const FName FDialogueBrowserTreeNode::GetParentVariableName() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentVariableName();
	}

	return NAME_None;
}

void FDialogueBrowserTreeNode::GetPathToChildThatContainsText(const TSharedPtr<Self>& Child,
	const FString& InSearch, TArray<TArray<TSharedPtr<Self>>>& OutNodes)
{
	// Child has text, build path to it
	bool bChildIsVisible;
	if (!Child->IsSeparator() && Child->DoesDisplayTextContains(InSearch))
	{
		bChildIsVisible = true;

		// Build path to top most parent
		TSharedPtr<Self> CurrentNode = Child;
		TArray<TSharedPtr<Self>> ChildOutNodes;
		while (CurrentNode->HasParent())
		{
			ChildOutNodes.Add(CurrentNode);

			// Parents are visible too
			TSharedPtr<Self> CurrentParentNode = CurrentNode->GetParent().Pin();
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
	for (const TSharedPtr<Self>& ChildItem : Child->GetChildren())
	{
		//const int32 NumBefore = OutNodes.Num();
		GetPathToChildThatContainsText(ChildItem, InSearch, OutNodes);

		ChildItem->SetIsVisible(bChildIsVisible && !ChildItem->IsSeparator() &&
								!ChildItem->IsCategory() && ChildItem->IsLeaf());
	}
}

void FDialogueBrowserTreeNode::FilterPathsToNodesThatContainText(const FString& InSearch,
	TArray<TArray<TSharedPtr<Self>>>& OutNodes)
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


FString FDialogueBrowserTreeNode::ToString() const
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

	AddKeyValueField("Text", DisplayText.ToString(), false);
	//AddFNameToOutput("ParticipantName", ParticipantName);
	//AddFNameToOutput("VariableName", VariableName);

	// if (Object.IsValid())
	// {
	// 	AddKeyValueField("Object Name", Object.Get()->GetName());
	// }
	AddKeyValueField("Children Num", FString::FromInt(Children.Num()));

	return Output + " }";
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeRootNode
FDialogueBrowserTreeRootNode::FDialogueBrowserTreeRootNode() :
	Super(FText::FromString(TEXT("ROOT")), nullptr)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeSeparatorNode
FDialogueBrowserTreeSeparatorNode::FDialogueBrowserTreeSeparatorNode(FDialogueBrowserTreeNodePtr InParent) :
	Super(FText::FromString(TEXT("SEPARATOR")), InParent)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeCategoryNode
FDialogueBrowserTreeCategoryNode::FDialogueBrowserTreeCategoryNode(const FText& InDisplayText,
	FDialogueBrowserTreeNodePtr InParent, const EDialogueTreeNodeCategoryType InCategoryType) :
	Super(InDisplayText, InParent)
{
	CategoryType = InCategoryType;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeParticipantNode
FDialogueBrowserTreeParticipantNode::FDialogueBrowserTreeParticipantNode(const FText& InDisplayText,
	FDialogueBrowserTreeNodePtr InParent, const FName& InParticipantName)
	: Super(InDisplayText, InParent), ParticipantName(InParticipantName)
{
}

const FName FDialogueBrowserTreeParticipantNode::GetParentParticipantName() const
{
	if (ParticipantName.IsValid() && !ParticipantName.IsNone())
	{
		return ParticipantName;
	}

	return Super::GetParentParticipantName();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeVariableNode
FDialogueBrowserTreeVariableNode::FDialogueBrowserTreeVariableNode(const FText& InDisplayText,
	FDialogueBrowserTreeNodePtr InParent, const FName& InVariableName)
	: Super(InDisplayText, InParent), VariableName(InVariableName)
{
}

const FName FDialogueBrowserTreeVariableNode::GetParentVariableName() const
{
	if (VariableName.IsValid() && !VariableName.IsNone())
	{
		return VariableName;
	}

	return Super::GetParentVariableName();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeCategoryParticipantNode
FDialogueBrowserTreeCategoryParticipantNode::FDialogueBrowserTreeCategoryParticipantNode(const FText& InDisplayText,
	FDialogueBrowserTreeNodePtr InParent, const FName& InParticipantName) :
	Super(InDisplayText, InParent, InParticipantName)
{
	CategoryType = EDialogueTreeNodeCategoryType::Participant;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeDialogueNode
FDialogueBrowserTreeDialogueNode::FDialogueBrowserTreeDialogueNode(const FText& InDisplayText,
	FDialogueBrowserTreeNodePtr InParent, const TWeakObjectPtr<const UDlgDialogue>& InObject) :
	Super(InDisplayText, InParent), Dialogue(InObject)
{
}

FReply FDialogueBrowserTreeDialogueNode::OnClick()
{
	if (Dialogue.IsValid())
	{
		FAssetEditorManager::Get().OpenEditorForAsset(const_cast<UDlgDialogue*>(Dialogue.Get()));
		return FReply::Handled();
	}

	return FReply::Unhandled();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeGraphNode
FDialogueBrowserTreeGraphNode::FDialogueBrowserTreeGraphNode(const FText& InDisplayText,
	FDialogueBrowserTreeNodePtr InParent, const TWeakObjectPtr<const UDialogueGraphNode>& InObject) :
	Super(InDisplayText, InParent), GraphNode(InObject)
{
}

FReply FDialogueBrowserTreeGraphNode::OnClick()
{
	if (GraphNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(Cast<UDialogueGraphNode_Base>(GraphNode.Get()))
				? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeEdgeNode
FDialogueBrowserTreeEdgeNode::FDialogueBrowserTreeEdgeNode(const FText& InDisplayText,
	FDialogueBrowserTreeNodePtr InParent, const TWeakObjectPtr<const UDialogueGraphNode_Edge>& InObject) :
	Super(InDisplayText, InParent), EdgeNode(InObject)
{
}

FReply FDialogueBrowserTreeEdgeNode::OnClick()
{
	if (EdgeNode.IsValid())
	{
		return FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(Cast<UDialogueGraphNode_Base>(EdgeNode.Get()))
				? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}
