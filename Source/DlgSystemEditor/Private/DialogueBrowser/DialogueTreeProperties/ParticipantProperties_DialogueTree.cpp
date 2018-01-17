// Fill out your copyright notice in the Description page of Project Settings.
#include "ParticipantProperties_DialogueTree.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueTreeParticipantProperties
void FParticipantProperties_DialogueTree::Sort()
{
	Dialogues.Sort(PredicateSortDialogueWeakPtrAlphabeticallyAscending);
	for (const auto& Pair : Events)
	{
		Pair.Value->Sort();
	}

	UDlgManager::SortDefault<FDlgTreeVariablePropertiesPtr>(Events);
	UDlgManager::SortDefault(Conditions);
	UDlgManager::SortDefault(Integers);
	UDlgManager::SortDefault(Floats);
	UDlgManager::SortDefault(Bools);
}
