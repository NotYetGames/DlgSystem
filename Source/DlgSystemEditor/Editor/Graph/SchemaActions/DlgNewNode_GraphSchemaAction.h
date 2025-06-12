// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "DlgSystem/Nodes/DlgNode.h"
#include "DlgSystem/NYEngineVersionHelpers.h"

#include "DlgNewNode_GraphSchemaAction.generated.h"

class UDialogueGraphNode;
class UDlgDialogue;
class UEdGraph;

/** Action to add a node to the graph */
USTRUCT()
struct DLGSYSTEMEDITOR_API FDlgNewNode_GraphSchemaAction : public FEdGraphSchemaAction
{
private:
	typedef FDlgNewNode_GraphSchemaAction Self;

public:
	GENERATED_USTRUCT_BODY();

	FDlgNewNode_GraphSchemaAction() : FEdGraphSchemaAction() {}
	FDlgNewNode_GraphSchemaAction(
		const FText& InNodeCategory,
		const FText& InMenuDesc, const FText& InToolTip,
		int32 InGrouping, TSubclassOf<UDlgNode> InCreateNodeType
	) : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), CreateNodeType(InCreateNodeType) {}

	//~ Begin FEdGraphSchemaAction Interface
	UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, FNYLocationVector2f Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface

	// Spawns a new UDialogueGraphNode of type GraphNodeType that must have a valid DialogueNode of TSubclassOf<UDlgNode>
	template <typename GraphNodeType>
	static GraphNodeType* SpawnGraphNodeWithDialogueNodeFromTemplate(
		UEdGraph* ParentGraph,
		TSubclassOf<UDlgNode> CreateNodeType,
		const FNYVector2f Location,
		bool bSelectNewNode = true
	)
	{
		Self Action(FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty(), 0, CreateNodeType);
		return CastChecked<GraphNodeType>(Action.PerformAction(ParentGraph, nullptr, Location, bSelectNewNode));
	}

private:
	/** Creates a new dialogue node from the template */
	UEdGraphNode* CreateNode(UDlgDialogue* Dialogue, UEdGraph* ParentGraph, UEdGraphPin* FromPin, FNYVector2f Location, bool bSelectNewNode);

	/** Connects new node to output of selected nodes */
//	void ConnectToSelectedNodes(UDialogueNode* NewNodeclass, UEdGraph* ParentGraph) const;

	// The node type used for when creating a new node (used by CreateNode)
	TSubclassOf<UDlgNode> CreateNodeType;
};

