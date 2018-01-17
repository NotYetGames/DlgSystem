// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgContext.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgNode.h"


const FText& UDlgContext::GetOptionText(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option = %d in GetOptionText!"), OptionIndex);
		return FText::GetEmpty();
	}

	return AvailableChildren[OptionIndex]->Text;
}

const FText& UDlgContext::GetActiveNodeText() const
{
	const UDlgNode* Node = GetActiveNode();

	if (Node == nullptr)
	{
		return FText::GetEmpty();
	}

	return Node->GetNodeText();
}

USoundWave* UDlgContext::GetActiveNodeVoiceSoundWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (Node == nullptr)
	{
		return nullptr;
	}

	return Node->GetNodeVoiceSoundWave();
}

UDialogueWave* UDlgContext::GetActiveNodeVoiceDialogueWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (Node == nullptr)
	{
		return nullptr;
	}

	return Node->GetNodeVoiceDialogueWave();
}

UObject* UDlgContext::GetActiveParticipant() const
{
	if (Dialogue == nullptr)
	{
		return nullptr;
	}

	const UDlgNode* Node = GetActiveNode();
	if (Node == nullptr)
	{
		return nullptr;
	}

	UObject* const* Item = Participants.Find(Node->GetNodeParticipantName());
	return Item == nullptr ? nullptr : *Item;
}


UObject* UDlgContext::GetParticipant(FName DlgParticipantName)
{
	UObject** ParticipantPtr = Participants.Find(DlgParticipantName);
	if (ParticipantPtr != nullptr)
	{
		return *ParticipantPtr;
	}

	return nullptr;
}
