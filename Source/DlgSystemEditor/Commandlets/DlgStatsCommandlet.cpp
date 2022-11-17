// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.

#include "DlgStatsCommandlet.h"
#include "DlgSystem/DlgManager.h"
#include "DlgSystem/DlgDialogue.h"
#include "DlgCommandletHelper.h"
#include "DlgSystem/Nodes/DlgNode_SpeechSequence.h"
#include "DlgSystem/Nodes/DlgNode_Speech.h"
#include "DlgSystem/DlgHelper.h"


DEFINE_LOG_CATEGORY(LogDlgStatsCommandlet);


UDlgStatsCommandlet::UDlgStatsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = false;
	ShowErrorCount = false;
}

int32 UDlgStatsCommandlet::Main(const FString& Params)
{
	UE_LOG(LogDlgStatsCommandlet, Display, TEXT("Starting"));

	// Parse command line - we're interested in the param vals
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> ParamVals;
	UCommandlet::ParseCommandLine(*Params, Tokens, Switches, ParamVals);

	UDlgManager::LoadAllDialoguesIntoMemory();
	const TArray<UDlgDialogue*> AllDialogues = UDlgManager::GetAllDialoguesFromMemory();

	FDlgStatsDialogue TotalStats;
	for (const UDlgDialogue* Dialogue : AllDialogues)
	{
		UPackage* Package = Dialogue->GetOutermost();
		check(Package);
		const FString OriginalDialoguePath = Package->GetPathName();

		// Only count game dialogues
		if (!FDlgHelper::IsPathInProjectDirectory(OriginalDialoguePath))
		{
			UE_LOG(LogDlgStatsCommandlet, Warning, TEXT("Dialogue = `%s` is not in the game directory, ignoring"), *OriginalDialoguePath);
			continue;
		}

		FDlgStatsDialogue DialogueStats;
		GetStatsForDialogue(*Dialogue, DialogueStats);
		TotalStats += DialogueStats;
		UE_LOG(LogDlgStatsCommandlet, Display, TEXT("Dialogue = %s. Total Text Word count = %d"), *OriginalDialoguePath, DialogueStats.WordCount);
	}

	UE_LOG(LogDlgStatsCommandlet, Display,
		LINE_TERMINATOR TEXT("Stats:") LINE_TERMINATOR
		TEXT("Total Text Word Count = %d"),
		TotalStats.WordCount);

	return 0;
}


bool UDlgStatsCommandlet::GetStatsForDialogue(const UDlgDialogue& Dialogue, FDlgStatsDialogue& OutStats)
{
	// Root
	for (const UDlgNode* StartNode : Dialogue.GetStartNodes())
	{
		OutStats.WordCount += GetNodeWordCount(*StartNode);
	}

	// Nodes
	const TArray<UDlgNode*>& Nodes = Dialogue.GetNodes();
	for (int32 NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++)
	{
		OutStats.WordCount += GetNodeWordCount(*Nodes[NodeIndex]);
	}

	return true;
}

int32 UDlgStatsCommandlet::GetNodeWordCount(const UDlgNode& Node) const
{
	const UDlgNode* NodePtr = &Node;
	int32 WordCount = 0;

	if (const UDlgNode_Speech* NodeSpeech = Cast<UDlgNode_Speech>(NodePtr))
	{
		WordCount += GetTextWordCount(NodeSpeech->GetNodeUnformattedText());
	}
	else if (const UDlgNode_SpeechSequence* NodeSpeechSequence = Cast<UDlgNode_SpeechSequence>(NodePtr))
	{
		// Speech Sequence
		for (const FDlgSpeechSequenceEntry& Entry : NodeSpeechSequence->GetNodeSpeechSequence())
		{
			WordCount += GetTextWordCount(Entry.Text);
		}
	}
	else
	{
		// not supported
	}

	// Edges
	for (const FDlgEdge& Edge : Node.GetNodeChildren())
	{
		WordCount += GetTextWordCount(Edge.GetUnformattedText());
	}

	return WordCount;
}

int32 UDlgStatsCommandlet::GetStringWordCount(const FString& String) const
{
	TArray<FString> Out;
	String.ParseIntoArray(Out, TEXT(" "), true);
	return Out.Num();
}
