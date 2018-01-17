// Fill out your copyright notice in the Description page of Project Settings.
#include "SGraphNode_DialogueRoot.h"

#include "DialogueGraphNode_Root.h"

void SGraphNode_DialogueRoot::Construct(const FArguments& InArgs, UDialogueGraphNode_Root* InNode)
{
	Super::Construct(Super::FArguments(), InNode);
	DialogueGraphNode_Root = InNode;
}
