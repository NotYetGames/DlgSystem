// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "Nodes/DlgNode.h"

#include "NewNode_DialogueGraphSchemaAction.generated.h"

class UDialogueGraphNode;
class UDlgDialogue;
class UEdGraph;

/** Action to add a node to the graph */
USTRUCT()
struct FNewNode_DialogueGraphSchemaAction : public FEdGraphSchemaAction
{
private:
	typedef FNewNode_DialogueGraphSchemaAction Self;

public:
	GENERATED_USTRUCT_BODY();

	FNewNode_DialogueGraphSchemaAction() : FEdGraphSchemaAction() {}
	FNewNode_DialogueGraphSchemaAction(
		const FText& InNodeCategory,
		const FText& InMenuDesc, const FText& InToolTip,
		int32 InGrouping, TSubclassOf<UDlgNode> InCreateNodeType
	) : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), CreateNodeType(InCreateNodeType) {}

	//~ Begin FEdGraphSchemaAction Interface
	UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface

	// Spawns a new UDialogueGraphNode of type GraphNodeType that must have a valid DialogueNode of TSubclassOf<UDlgNode>
	template <typename GraphNodeType>
	static GraphNodeType* SpawnGraphNodeWithDialogueNodeFromTemplate(
		UEdGraph* ParentGraph,
		TSubclassOf<UDlgNode> CreateNodeType,
		const FVector2D Location,
		bool bSelectNewNode = true
	)
	{
		Self Action(FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty(), 0, CreateNodeType);
		return CastChecked<GraphNodeType>(Action.PerformAction(ParentGraph, nullptr, Location, bSelectNewNode));
	}

private:
	/** Creates a new dialogue node from the template */
	UEdGraphNode* CreateNode(UDlgDialogue* Dialogue, UEdGraph* ParentGraph, UEdGraphPin* FromPin, FVector2D Location, bool bSelectNewNode);

	/** Connects new node to output of selected nodes */
//	void ConnectToSelectedNodes(UDialogueNode* NewNodeclass, UEdGraph* ParentGraph) const;

	// The node type used for when creating a new node (used by CreateNode)
	TSubclassOf<UDlgNode> CreateNodeType;
};

