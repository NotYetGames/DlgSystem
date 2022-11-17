// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreTypes.h"

#include "DialogueGraphNode.h"

#include "DialogueGraphNode_Root.generated.h"


UCLASS()
class DLGSYSTEMEDITOR_API UDialogueGraphNode_Root : public UDialogueGraphNode
{
	GENERATED_BODY()

public:
	// Begin UEdGraphNode interface
	/** Gets the name of this node, shown in title bar */
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	/** Gets the tooltip to display when over the node */
	FText GetTooltipText() const override
	{
		return NSLOCTEXT("DialogueGraphNode_Root", "RootToolTip", "The root start node of this graph");
	}

	/** Called when the connection list of one of the pins of this node is changed in the editor */
	void PinConnectionListChanged(UEdGraphPin* Pin) override;

	// Begin UDialogueGraphNode interface
	bool IsRootNode() const override { return true; }

	/** Sets the Dialogue node index number, this represents the index from the DlgDialogue.Nodes Array */
	void SetDialogueNodeIndex(int32 InIndex) override { NodeIndex = INDEX_NONE; }

	/** Gets the Dialogue node index number for the DlgDialogue.Nodes Array */
	int32 GetDialogueNodeIndex() const override { return INDEX_NONE; }

	/** Gets the background color of this node. */
	FLinearColor GetNodeBackgroundColor() const override { return GetDefault<UDlgSystemSettings>()->RootNodeColor; }

protected:
	// Begin UDialogueGraphNode interface
	/** This function is called after one of the properties of the Dialogue are changed. */
	void OnDialoguePropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent) override;
};
