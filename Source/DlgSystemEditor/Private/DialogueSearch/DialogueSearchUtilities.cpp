// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueSearchUtilities.h"

#include "DialogueEditor/Graph/DialogueGraph.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchUtilities
FDialogueSearchFoundResultPtr FDialogueSearchUtilities::GetGraphNodesForEventEventName(
		const FName& EventName, const UDlgDialogue* Dialogue)
{
	FDialogueSearchFoundResultPtr FoundResult = FDialogueSearchFoundResult::Make();

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	for (UDialogueGraphNode* GraphNode : Graph->GetAllDialogueGraphNodes())
	{
		// Enter events
		if (IsEventInArray(EventName, EDlgEventType::DlgEventEvent, GraphNode->GetDialogueNode().GetNodeEnterEvents()))
		{
			FoundResult->GraphNodes.Add(GraphNode);
		}
	}

	return FoundResult;
}

FDialogueSearchFoundResultPtr FDialogueSearchUtilities::GetGraphNodesForConditionEventCallName(
		const FName& ConditionName, const UDlgDialogue* Dialogue)
{
	FDialogueSearchFoundResultPtr FoundResult = FDialogueSearchFoundResult::Make();

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	for (UDialogueGraphNode_Base* GraphNodeBase : Graph->GetAllBaseDialogueGraphNodes())
	{
		if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(GraphNodeBase))
		{
			// Node
			// Node Enter conditions
			if (IsConditionInArray(ConditionName, EDlgConditionType::DlgConditionEventCall,
								   GraphNode->GetDialogueNode().GetNodeEnterConditions()))
			{
				FoundResult->GraphNodes.Add(GraphNode);
			}

			// The children are handled by the edges, below
		}
		else if (UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(GraphNodeBase))
		{
			// Edge
			if (IsConditionInArray(ConditionName, EDlgConditionType::DlgConditionEventCall,
								   EdgeNode->GetDialogueEdge().Conditions))
			{
				FoundResult->EdgeNodes.Add(EdgeNode);
			}
		}
	}

	return FoundResult;
}

FDialogueSearchFoundResultPtr FDialogueSearchUtilities::GetGraphNodesForVariablesOfNameAndType(const FName& VariableName,
	const UDlgDialogue* Dialogue, const EDlgEventType EventType, const EDlgConditionType ConditionType)
{
	FDialogueSearchFoundResultPtr FoundResult = FDialogueSearchFoundResult::Make();

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	for (UDialogueGraphNode_Base* GraphNodeBase : Graph->GetAllBaseDialogueGraphNodes())
	{
		if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(GraphNodeBase))
		{
			// The root node does not have searchable info
			if (GraphNode->IsRootNode())
			{
				continue;
			}

			// Node
			const UDlgNode& Node = GraphNode->GetDialogueNode();

			// Enter events
			if (IsEventInArray(VariableName, EventType, Node.GetNodeEnterEvents()))
			{
				FoundResult->GraphNodes.Add(GraphNode);
			}

			// Enter conditions
			if (IsConditionInArray(VariableName, ConditionType, Node.GetNodeEnterConditions()))
			{
				FoundResult->GraphNodes.Add(GraphNode);
			}

			// The children are handled by the edges, below
		}
		else if (UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(GraphNodeBase))
		{
			// Edge
			if (IsConditionInArray(VariableName, ConditionType,
								   EdgeNode->GetDialogueEdge().Conditions))
			{
				FoundResult->EdgeNodes.Add(EdgeNode);
			}
		}
	}

	return FoundResult;
}
