// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueGraphNode_Root.h"

#include "DialogueGraphNode.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UObject interface
bool UDialogueGraphNode_Root::CanEditChange(const UProperty* InProperty) const
{
	check(IsRootNode());
	const bool bIsEditable = Super::CanEditChange(InProperty);
	return bIsEditable;
}
// End UObject interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	if (!IsValid(PropertyChangedEvent.Property) || !IsValid(PropertyChangedEvent.MemberProperty))
	{
		return;
	}

	// Only intrested in the Dialogue.StartNode
	if (PropertyChangedEvent.MemberProperty->GetFName() != UDlgDialogue::GetMemberNameStartNode())
	{
		return;
	}

	//HandleOnDialoguePropertyChanged(PropertyChangedEvent, GetDialogue()->GetStartNode());
}
// End UDialogueGraphNode_Base interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
