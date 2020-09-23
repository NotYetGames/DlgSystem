// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueBrowserTreeNode.h"

#include "Widgets/Views/STreeView.h"

#include "DialogueEditorUtilities.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeNode
FDialogueBrowserTreeNode::FDialogueBrowserTreeNode(const FText& InDisplayText, const TSharedPtr<Self>& InParent)
	: Super(InDisplayText, InParent)
{
}

FName FDialogueBrowserTreeNode::GetParentParticipantName() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentParticipantName();
	}

	return NAME_None;
}

FName FDialogueBrowserTreeNode::GetParentVariableName() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentVariableName();
	}

	return NAME_None;
}

UClass* FDialogueBrowserTreeNode::GetParentClass() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentClass();
	}

	return nullptr;
}

FString FDialogueBrowserTreeNode::ToString() const
{
	// static const ANSICHAR* EDialogueTreeItemCategoryTypeStringMap[] = {
	// 	"Default",
	// 	"Participant",
	// 	"Dialogue",
	// 	"Event",
	// 	"Condition",
	// 	"Variable",
	// 	"VariableInt",
	// 	"VariableFloat",
	// 	"VariableBool",
	// 	"Max"
	// };

	FString Output = "FTextItem { ";

	auto AddKeyValueField = [&Output](const FString& Key, const FString& Value, const bool bAddSeparator = true)
	{
		if (bAddSeparator)
		{
			Output += " | ";
		}
		Output += Key + " = `" + Value + "`";
	};
	auto AddFNameToOutput = [&Output, &AddKeyValueField](const FString& Name, FName Value)
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
FDialogueBrowserTreeSeparatorNode::FDialogueBrowserTreeSeparatorNode(
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent)
	: Super(FText::FromString(TEXT("SEPARATOR")), InParent)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeCategoryNode
FDialogueBrowserTreeCategoryNode::FDialogueBrowserTreeCategoryNode(
	const FText& InDisplayText,
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
	EDialogueTreeNodeCategoryType InCategoryType
) : Super(InDisplayText, InParent)
{
	CategoryType = InCategoryType;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeParticipantNode
FDialogueBrowserTreeParticipantNode::FDialogueBrowserTreeParticipantNode(
	const FText& InDisplayText,
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
	FName InParticipantName
) : Super(InDisplayText, InParent), ParticipantName(InParticipantName)
{
}

FName FDialogueBrowserTreeParticipantNode::GetParentParticipantName() const
{
	if (ParticipantName.IsValid() && !ParticipantName.IsNone())
	{
		return ParticipantName;
	}

	return Super::GetParentParticipantName();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeVariableNode
FDialogueBrowserTreeVariableNode::FDialogueBrowserTreeVariableNode(
	const FText& InDisplayText,
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
	FName InVariableName
) : Super(InDisplayText, InParent), VariableName(InVariableName)
{
}

FName FDialogueBrowserTreeVariableNode::GetParentVariableName() const
{
	if (VariableName.IsValid() && !VariableName.IsNone())
	{
		return VariableName;
	}

	return Super::GetParentVariableName();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeCustomObjectNode

FDialogueBrowserTreeCustomObjectNode::FDialogueBrowserTreeCustomObjectNode(
    const FText& InDisplayText,
    const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
    UClass* ObjectClass
) : Super(InDisplayText, InParent), Class(ObjectClass)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeCategoryParticipantNode
FDialogueBrowserTreeCategoryParticipantNode::FDialogueBrowserTreeCategoryParticipantNode(
	const FText& InDisplayText,
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
	FName InParticipantName
) : Super(InDisplayText, InParent, InParticipantName)
{
	CategoryType = EDialogueTreeNodeCategoryType::Participant;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeDialogueNode
FDialogueBrowserTreeDialogueNode::FDialogueBrowserTreeDialogueNode(
	const FText& InDisplayText,
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
	const TWeakObjectPtr<const UDlgDialogue>& InObject
) : Super(InDisplayText, InParent), Dialogue(InObject)
{
}

FReply FDialogueBrowserTreeDialogueNode::OnClick()
{
	if (Dialogue.IsValid())
	{
		FDialogueEditorUtilities::OpenEditorForAsset(Dialogue.Get());
		return FReply::Handled();
	}

	return FReply::Unhandled();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeGraphNode
FDialogueBrowserTreeGraphNode::FDialogueBrowserTreeGraphNode(
	const FText& InDisplayText,
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
	const TWeakObjectPtr<const UDialogueGraphNode>& InObject
) : Super(InDisplayText, InParent), GraphNode(InObject)
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
FDialogueBrowserTreeEdgeNode::FDialogueBrowserTreeEdgeNode(
	const FText& InDisplayText,
	const TSharedPtr<FDialogueBrowserTreeNode>& InParent,
	const TWeakObjectPtr<const UDialogueGraphNode_Edge>& InObject
) : Super(InDisplayText, InParent), EdgeNode(InObject)
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
