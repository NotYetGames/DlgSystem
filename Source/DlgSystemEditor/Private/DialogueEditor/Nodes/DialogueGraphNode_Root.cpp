// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueGraphNode_Root.h"

#include "DialogueGraphNode.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UEdGraphNode interface

FText UDialogueGraphNode_Root::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const TArray<UDlgNode*> StartNodes = GetDialogue()->GetStartNodes();
	if (StartNodes.Num() == 1)
	{
		return NSLOCTEXT("DialogueGraphNode_Root", "RootTitle", "Start");
	}

	const int32 StartNodeIndex = StartNodes.Find(DialogueNode);
	const FString AsString = FString("Start ") + FString::FromInt(StartNodeIndex);
	return FText::FromString(AsString);
}

void UDialogueGraphNode_Root::PinConnectionListChanged(UEdGraphPin* Pin)
{
	// Root node can only have output nodes so any other type should not be supported.
	if (Pin->Direction == EGPD_Output)
	{
		Super::PinConnectionListChanged(Pin);
	}
}
// End UEdGraphNode interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UDialogueGraphNode_Base interface
void UDialogueGraphNode_Root::OnDialoguePropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!PropertyChangedEvent.Property || !PropertyChangedEvent.MemberProperty)
	{
		return;
	}

	//HandleOnDialoguePropertyChanged(PropertyChangedEvent, GetDialogue()->GetStartNode());
}
// End UDialogueGraphNode_Base interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
