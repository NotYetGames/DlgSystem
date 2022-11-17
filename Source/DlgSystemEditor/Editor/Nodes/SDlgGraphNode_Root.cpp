// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgGraphNode_Root.h"

#include "DialogueGraphNode_Root.h"

void SDlgGraphNode_Root::Construct(const FArguments& InArgs, UDialogueGraphNode_Root* InNode)
{
	Super::Construct(Super::FArguments(), InNode);
	DialogueGraphNode_Root = InNode;
}
