// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgBrowserTreeNode.h"

#include "DlgSystemEditor/DlgEditorUtilities.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgBrowserTreeNode
FDlgBrowserTreeNode::FDlgBrowserTreeNode(const FText& InDisplayText, const TSharedPtr<Self>& InParent)
	: Super(InDisplayText, InParent)
{
}

FName FDlgBrowserTreeNode::GetParentParticipantName() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentParticipantName();
	}

	return NAME_None;
}

FName FDlgBrowserTreeNode::GetParentVariableName() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentVariableName();
	}

	return NAME_None;
}

UClass* FDlgBrowserTreeNode::GetParentClass() const
{
	if (Parent.IsValid())
	{
		return Parent.Pin()->GetParentClass();
	}

	return nullptr;
}

FString FDlgBrowserTreeNode::ToString() const
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
	const TSharedPtr<FDlgBrowserTreeNode>& InParent)
	: Super(FText::FromString(TEXT("SEPARATOR")), InParent)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeCategoryNode
FDialogueBrowserTreeCategoryNode::FDialogueBrowserTreeCategoryNode(
	const FText& InDisplayText,
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
	EDlgTreeNodeCategoryType InCategoryType
) : Super(InDisplayText, InParent)
{
	CategoryType = InCategoryType;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeParticipantNode
FDialogueBrowserTreeParticipantNode::FDialogueBrowserTreeParticipantNode(
	const FText& InDisplayText,
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
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
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
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
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
	UClass* ObjectClass
) : Super(InDisplayText, InParent), Class(ObjectClass)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeCategoryParticipantNode
FDialogueBrowserTreeCategoryParticipantNode::FDialogueBrowserTreeCategoryParticipantNode(
	const FText& InDisplayText,
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
	FName InParticipantName
) : Super(InDisplayText, InParent, InParticipantName)
{
	CategoryType = EDlgTreeNodeCategoryType::Participant;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeDialogueNode
FDialogueBrowserTreeDialogueNode::FDialogueBrowserTreeDialogueNode(
	const FText& InDisplayText,
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
	const TWeakObjectPtr<const UDlgDialogue>& InObject
) : Super(InDisplayText, InParent), Dialogue(InObject)
{
}

FReply FDialogueBrowserTreeDialogueNode::OnClick()
{
	if (Dialogue.IsValid())
	{
		FDlgEditorUtilities::OpenEditorForAsset(Dialogue.Get());
		return FReply::Handled();
	}

	return FReply::Unhandled();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeGraphNode
FDialogueBrowserTreeGraphNode::FDialogueBrowserTreeGraphNode(
	const FText& InDisplayText,
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
	const TWeakObjectPtr<const UDialogueGraphNode>& InObject
) : Super(InDisplayText, InParent), GraphNode(InObject)
{
}

FReply FDialogueBrowserTreeGraphNode::OnClick()
{
	if (GraphNode.IsValid())
	{
		return FDlgEditorUtilities::OpenEditorAndJumpToGraphNode(Cast<UDialogueGraphNode_Base>(GraphNode.Get()))
				? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeEdgeNode
FDialogueBrowserTreeEdgeNode::FDialogueBrowserTreeEdgeNode(
	const FText& InDisplayText,
	const TSharedPtr<FDlgBrowserTreeNode>& InParent,
	const TWeakObjectPtr<const UDialogueGraphNode_Edge>& InObject
) : Super(InDisplayText, InParent), EdgeNode(InObject)
{
}

FReply FDialogueBrowserTreeEdgeNode::OnClick()
{
	if (EdgeNode.IsValid())
	{
		return FDlgEditorUtilities::OpenEditorAndJumpToGraphNode(Cast<UDialogueGraphNode_Base>(EdgeNode.Get()))
				? FReply::Handled() : FReply::Unhandled();
	}

	return FReply::Unhandled();
}
