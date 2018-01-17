// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "EdGraphUtilities.h"

// Factory for creating Dialogue graph nodes
// WARNING: UEdGraphNode::CreateVisualWidget has priority over this, see FNodeFactory::CreateNodeWidget
struct FDialogueGraphNodeFactory : public FGraphPanelNodeFactory
{
	TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* InNode) const override;
};

// Factory  for creating pin widgets
// This is the highest priority creator, see FNodeFactory::CreatePinWidget
struct FDialogueGraphPinFactory : public FGraphPanelPinFactory
{
public:
	TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* Pin) const override;
};

// Factory for creating the drawinng policy between nodes.
// Defined in UDialogueGraphSchema::CreateConnectionDrawingPolicy which has priority over FGraphPanelPinConnectionFactory,
// see FNodeFactory::CreateConnectionPolicy
