// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "Nodes/DlgNode_SpeechSequence.h"
#include "DlgContextInternal.h"


#if WITH_EDITOR
void UDlgNode_SpeechSequence::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// fill edges automatically based on input data
	AutoGenerateInnerEdges();
}
#endif

bool UDlgNode_SpeechSequence::HandleNodeEnter(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	ActualIndex = 0;
	return Super::HandleNodeEnter(DlgContext, NodesEnteredWithThisStep);
}

bool UDlgNode_SpeechSequence::ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyEvaluated)
{
	TArray<const FDlgEdge*>& Options = DlgContext->GetOptionArray();
	TArray<FDlgEdgeData>& AllOptions = DlgContext->GetAllOptionsArray();
	Options.Empty();
	AllOptions.Empty();

	// If the last entry is active the real edges are used
	if (ActualIndex == SpeechSequence.Num() - 1)
		return Super::ReevaluateChildren(DlgContext, AlreadyEvaluated);

	// give the context the fake inner edge
	if (InnerEdges.IsValidIndex(ActualIndex))
	{
		Options.Add(&InnerEdges[ActualIndex]);
		AllOptions.Add({ true, &InnerEdges[ActualIndex] });
		return true;
	}

	return false;
}

bool UDlgNode_SpeechSequence::OptionSelected(int32 OptionIndex, UDlgContextInternal* DlgContext)
{
	// Actual index is valid, and not the last node in the speech sequence, increment
	if (ActualIndex >= 0 && ActualIndex < SpeechSequence.Num() - 1)
	{
		ActualIndex += 1;
		return ReevaluateChildren(DlgContext, {this});
	}

	ActualIndex = 0;
	// node finished -> generate true children
	Super::ReevaluateChildren(DlgContext, { this });
	return Super::OptionSelected(OptionIndex, DlgContext);
}

const FText& UDlgNode_SpeechSequence::GetNodeText() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].Text;
	}

	return FText::GetEmpty();
}

USoundWave* UDlgNode_SpeechSequence::GetNodeVoiceSoundWave() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].VoiceSoundWave;
	}

	return nullptr;
}

UDialogueWave* UDlgNode_SpeechSequence::GetNodeVoiceDialogueWave() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].VoiceDialogueWave;
	}

	return nullptr;
}

FName UDlgNode_SpeechSequence::GetSpeakerState() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].SpeakerState;
	}

	return NAME_None;
}

void UDlgNode_SpeechSequence::AddSpeakerStates(TSet<FName>& States) const
{
	for (const auto& SpeechEntry : SpeechSequence)
	{
		States.Add(SpeechEntry.SpeakerState);
	}
}

FName UDlgNode_SpeechSequence::GetNodeParticipantName() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].Speaker;
	}

	return OwnerName;
}

void UDlgNode_SpeechSequence::GetAssociatedParticipants(TArray<FName>& OutArray) const
{
	Super::GetAssociatedParticipants(OutArray);

	for (const FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		if (Entry.Speaker != NAME_None)
		{
			OutArray.AddUnique(Entry.Speaker);
		}
	}
}

void UDlgNode_SpeechSequence::AutoGenerateInnerEdges()
{
	InnerEdges.Empty();
	for (const FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		FDlgEdge Edge;
		Edge.Text = Entry.EdgeText;
		InnerEdges.Add(Edge);
	}
}
