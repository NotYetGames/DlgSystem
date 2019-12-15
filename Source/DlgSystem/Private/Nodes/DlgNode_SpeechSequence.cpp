// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "Nodes/DlgNode_SpeechSequence.h"
#include "DlgContextInternal.h"
#include "DlgLocalizationHelper.h"


#if WITH_EDITOR
void UDlgNode_SpeechSequence::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// fill edges automatically based on input data
	AutoGenerateInnerEdges();
}
#endif

void UDlgNode_SpeechSequence::UpdateTextsValuesFromDefaultsAndRemappings(const UDlgSystemSettings* Settings, bool bEdges, bool bUpdateGraphNode)
{
	for (FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		// We only care about edges here
		if (Settings->bSetDefaultEdgeTexts)
		{
			// Inner edges always point to a normal node and are always the unique edge child
			if (Entry.EdgeText.IsEmpty())
			{
				Entry.EdgeText = Settings->DefaultTextEdgeToNormalNode;
			}
		}

		FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Entry.Text);
		FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Entry.EdgeText);
	}
	Super::UpdateTextsValuesFromDefaultsAndRemappings(Settings, bEdges, bUpdateGraphNode);
}

void UDlgNode_SpeechSequence::UpdateTextsNamespacesAndKeys(const UDlgSystemSettings* Settings, bool bEdges, bool bUpdateGraphNode)
{
	UObject* Outer = GetOuter();
	if (!IsValid(Outer))
	{
		return;
	}
	
	for (FDlgSpeechSequenceEntry& Entry : SpeechSequence)
	{
		FDlgLocalizationHelper::UpdateTextNamespaceAndKey(Outer, Settings, Entry.Text);
		FDlgLocalizationHelper::UpdateTextNamespaceAndKey(Outer, Settings, Entry.EdgeText);
	}

	Super::UpdateTextsNamespacesAndKeys(Settings, bEdges, bUpdateGraphNode);
}

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

UDlgNodeData* UDlgNode_SpeechSequence::GetNodeData() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].NodeData;
	}

	return nullptr;
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

UObject* UDlgNode_SpeechSequence::GetGenericData() const
{
	if (SpeechSequence.IsValidIndex(ActualIndex))
	{
		return SpeechSequence[ActualIndex].GenericData;
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

void UDlgNode_SpeechSequence::AddAllSpeakerStatesIntoSet(TSet<FName>& OutStates) const
{
	for (const auto& SpeechEntry : SpeechSequence)
	{
		OutStates.Add(SpeechEntry.SpeakerState);
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
		Edge.SetUnformattedText(Entry.EdgeText);
		InnerEdges.Add(Edge);
	}
}
