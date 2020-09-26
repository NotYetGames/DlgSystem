// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueGraphFactories.h"

#include "K2Node.h"

#include "DialogueEditor/Nodes/DialogueGraphNode_Base.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Root.h"
#include "DialogueEditor/Nodes/SDialogueGraphNode.h"
#include "DialogueEditor/Nodes/SDialogueGraphNode_Root.h"
#include "DialogueEditor/Nodes/SDialogueGraphNode_Edge.h"
#include "DialogueEditor/Nodes/SDialogueGraphPin.h"
#include "BlueprintNodes/DialogueK2Node_Select.h"
#include "BlueprintNodes/SGraphNode_DialogueK2Select.h"
#include "BlueprintNodes/SGraphNode_DialogueK2Switch.h"
#include "BlueprintNodes/DialogueK2Node_SwitchDialogueCallback.h"

TSharedPtr<class SGraphNode> FDialogueGraphNodeFactory::CreateNode(class UEdGraphNode* InNode) const
{
	// Dialogue Editor Nodes
	if (UDialogueGraphNode_Base* DialogueNode_Base = Cast<UDialogueGraphNode_Base>(InNode))
	{
		// Nodes
		if (UDialogueGraphNode* DialogueNode = Cast<UDialogueGraphNode>(DialogueNode_Base))
		{
			if (UDialogueGraphNode_Root* DialogueStartNode = Cast<UDialogueGraphNode_Root>(DialogueNode))
			{
				return SNew(SDialogueGraphNode_Root, DialogueStartNode);
			}

			return SNew(SDialogueGraphNode, DialogueNode);
		}

		// Edge
		if (UDialogueGraphNode_Edge* DialogueEdge = Cast<UDialogueGraphNode_Edge>(DialogueNode_Base))
		{
			return SNew(SDialogueGraphNode_Edge, DialogueEdge);
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

TSharedPtr<class SGraphPin> FDialogueGraphPinFactory::CreatePin(class UEdGraphPin* Pin) const
{
	if (Pin->GetSchema()->IsA<UDialogueGraphSchema>())
	{
		return SNew(SDialogueGraphPin, Pin);
	}

	return nullptr;
}
