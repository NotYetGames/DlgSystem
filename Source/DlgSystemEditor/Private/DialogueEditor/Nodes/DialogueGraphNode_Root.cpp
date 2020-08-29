// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueGraphNode_Root.h"

#include "DialogueGraphNode.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UEdGraphNode interface
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

	// Only interested in the Dialogue.StartNode
	if (PropertyChangedEvent.MemberProperty->GetFName() != UDlgDialogue::GetMemberNameStartNode())
	{
		return;
	}

	//HandleOnDialoguePropertyChanged(PropertyChangedEvent, GetDialogue()->GetStartNode());
}
// End UDialogueGraphNode_Base interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
