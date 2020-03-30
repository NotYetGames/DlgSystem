// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"

#include "SDialogueGraphNode.h"

class UDialogueGraphNode_Root;

/**
 * Widget for UDialogueGraphNode_Root
 */
class SDialogueGraphNode_Root : public SDialogueGraphNode
{
	typedef SDialogueGraphNode Super;
public:

	SLATE_BEGIN_ARGS(SDialogueGraphNode_Root) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogueGraphNode_Root* InNode);
private:
	// The dialogue root this view represents
	UDialogueGraphNode_Root* DialogueGraphNode_Root = nullptr;
};
