// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueSearchUtilities.h"

#include "DialogueEditor/Graph/DialogueGraph.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueSearchUtilities
TSharedPtr<FDialogueSearchFoundResult> FDialogueSearchUtilities::GetGraphNodesForEventEventName(
		const FName& EventName, const UDlgDialogue* Dialogue)
{
	TSharedPtr<FDialogueSearchFoundResult> FoundResult = FDialogueSearchFoundResult::Make();

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	for (const UDialogueGraphNode* GraphNode : Graph->GetAllDialogueGraphNodes())
	{
		// Enter events
		if (IsEventInArray(EventName, EDlgEventType::Event, GraphNode->GetDialogueNode().GetNodeEnterEvents()))
		{
			FoundResult->GraphNodes.Add(GraphNode);
		}
	}

	return FoundResult;
}

TSharedPtr<FDialogueSearchFoundResult> FDialogueSearchUtilities::GetGraphNodesForConditionEventCallName(
		const FName& ConditionName, const UDlgDialogue* Dialogue)
{
	TSharedPtr<FDialogueSearchFoundResult> FoundResult = FDialogueSearchFoundResult::Make();

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	for (const UDialogueGraphNode_Base* GraphNodeBase : Graph->GetAllBaseDialogueGraphNodes())
	{
		if (const UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(GraphNodeBase))
		{
			// Node
			// Node Enter conditions
			if (IsConditionInArray(ConditionName, EDlgConditionType::EventCall,
								   GraphNode->GetDialogueNode().GetNodeEnterConditions()))
			{
				FoundResult->GraphNodes.Add(GraphNode);
			}

			// The children are handled by the edges, below
		}
		else if (const UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(GraphNodeBase))
		{
			// Edge
			if (IsConditionInArray(ConditionName, EDlgConditionType::EventCall,
								   EdgeNode->GetDialogueEdge().Conditions))
			{
				FoundResult->EdgeNodes.Add(EdgeNode);
			}
		}
	}

	return FoundResult;
}

TSharedPtr<FDialogueSearchFoundResult> FDialogueSearchUtilities::GetGraphNodesForVariablesOfNameAndType(const FName& VariableName,
	const UDlgDialogue* Dialogue, const EDlgEventType EventType, const EDlgConditionType ConditionType)
{
	TSharedPtr<FDialogueSearchFoundResult> FoundResult = FDialogueSearchFoundResult::Make();

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	for (const UDialogueGraphNode_Base* GraphNodeBase : Graph->GetAllBaseDialogueGraphNodes())
	{
		if (const UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(GraphNodeBase))
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
		else if (const UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(GraphNodeBase))
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


void FDialogueSearchUtilities::GetGraphNodesForTextArgumentVariable(const FName& VariableName,
	const UDlgDialogue* Dialogue, const EDlgTextArgumentType ArgumentType, TSharedPtr<FDialogueSearchFoundResult>& FoundResult)
{

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	for (const UDialogueGraphNode_Base* GraphNodeBase : Graph->GetAllBaseDialogueGraphNodes())
	{
		if (const UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(GraphNodeBase))
		{
			// The root node does not have searchable info
			if (GraphNode->IsRootNode())
			{
				continue;
			}

			// Node
			const UDlgNode& Node = GraphNode->GetDialogueNode();

			if (IsTextArgumentInArray(VariableName, ArgumentType, Node.GetTextArguments()))
			{
				FoundResult->GraphNodes.Add(GraphNode);
			}
		}
		else if (const UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(GraphNodeBase))
		{
			// Edge
			if (IsTextArgumentInArray(VariableName, ArgumentType, EdgeNode->GetDialogueEdge().GetTextArguments()))
			{
				FoundResult->EdgeNodes.Add(EdgeNode);
			}
		}
	}
}
