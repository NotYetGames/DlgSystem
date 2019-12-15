// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "SDialogueGraphNode_Root.h"

#include "DialogueGraphNode_Root.h"

void SDialogueGraphNode_Root::Construct(const FArguments& InArgs, UDialogueGraphNode_Root* InNode)
{
	Super::Construct(Super::FArguments(), InNode);
	DialogueGraphNode_Root = InNode;
}
