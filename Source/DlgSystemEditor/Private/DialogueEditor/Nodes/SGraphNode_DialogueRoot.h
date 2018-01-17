// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"

#include "SGraphNode_Dialogue.h"

class UDialogueGraphNode_Root;

/**
 * Widget for UDialogueGraphNode_Root
 */
class SGraphNode_DialogueRoot : public SGraphNode_DialogueNode
{
	typedef SGraphNode_DialogueNode Super;
public:

	SLATE_BEGIN_ARGS(SGraphNode_DialogueRoot) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogueGraphNode_Root* InNode);
private:
	// The dialogue root this view represents
	UDialogueGraphNode_Root* DialogueGraphNode_Root = nullptr;
};
