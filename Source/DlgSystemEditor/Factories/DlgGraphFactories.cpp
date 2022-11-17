// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgGraphFactories.h"

#include "K2Node.h"

#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Base.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Root.h"
#include "DlgSystemEditor/Editor/Nodes/SDlgGraphNode.h"
#include "DlgSystemEditor/Editor/Nodes/SDlgGraphNode_Root.h"
#include "DlgSystemEditor/Editor/Nodes/SDlgGraphNode_Edge.h"
#include "DlgSystemEditor/Editor/Nodes/SDlgGraphPin.h"
#include "DlgSystemEditor/BlueprintNodes/DialogueK2Node_Select.h"
#include "DlgSystemEditor/BlueprintNodes/SGraphNode_DialogueK2Select.h"
#include "DlgSystemEditor/BlueprintNodes/SGraphNode_DialogueK2Switch.h"
#include "DlgSystemEditor/BlueprintNodes/DialogueK2Node_SwitchDialogueCallback.h"

TSharedPtr<class SGraphNode> FDlgGraphNodeFactory::CreateNode(class UEdGraphNode* InNode) const
{
	// Dialogue Editor Nodes
	if (UDialogueGraphNode_Base* DialogueNode_Base = Cast<UDialogueGraphNode_Base>(InNode))
	{
		// Nodes
		if (UDialogueGraphNode* DialogueNode = Cast<UDialogueGraphNode>(DialogueNode_Base))
		{
			if (UDialogueGraphNode_Root* DialogueStartNode = Cast<UDialogueGraphNode_Root>(DialogueNode))
			{
				return SNew(SDlgGraphNode_Root, DialogueStartNode);
			}

			return SNew(SDlgGraphNode, DialogueNode);
		}

		// Edge
		if (UDialogueGraphNode_Edge* DialogueEdge = Cast<UDialogueGraphNode_Edge>(DialogueNode_Base))
		{
			return SNew(SDlgGraphNode_Edge, DialogueEdge);
		}
	}

	// Blueprint Editor Nodes
	if (UK2Node* BlueprintNode_Base = Cast<UK2Node>(InNode))
	{
		if (UDialogueK2Node_Select* BlueprintDialogueSelectNode = Cast<UDialogueK2Node_Select>(BlueprintNode_Base))
		{
			return SNew(SGraphNode_DialogueK2Select, BlueprintDialogueSelectNode);
		}

		if (UDialogueK2Node_SwitchDialogueCallback* BlueprintDialogueSwitchNode = Cast<UDialogueK2Node_SwitchDialogueCallback>(BlueprintNode_Base))
		{
			return SNew(SGraphNode_DialogueK2Switch, BlueprintDialogueSwitchNode);
		}
	}

	return nullptr;
}

TSharedPtr<class SGraphPin> FDlgGraphPinFactory::CreatePin(class UEdGraphPin* Pin) const
{
	if (Pin->GetSchema()->IsA<UDialogueGraphSchema>())
	{
		return SNew(SDlgGraphPin, Pin);
	}

	return nullptr;
}
