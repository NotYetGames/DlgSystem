// Copyright 2017-2018 Csaba Molnar, Daniel Butum
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
	UDlgManager::SortDefault(FNames);
}
