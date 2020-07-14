// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgMemory.h"
#include "DlgHelper.h"

bool FDlgHistory::operator==(const FDlgHistory& Other) const
{
	return FDlgHelper::IsSetEqual(VisitedNodeIndices, Other.VisitedNodeIndices);
}

void FDlgMemory::SetEntry(const FGuid& DialogueGUID, const FDlgHistory& History)
{
	FDlgHistory* OldEntry = HistoryMap.Find(DialogueGUID);

	if (OldEntry == nullptr)
	{
		HistoryMap.Add(DialogueGUID, History);
	}
	else
	{
		*OldEntry = History;
	}
}

void FDlgMemory::SetNodeVisited(const FGuid& DialogueGUID, int32 NodeIndex)
{
	FDlgHistory* History = HistoryMap.Find(DialogueGUID);
	// Add it if it does not exist already
	if (History == nullptr)
	{
		History = &HistoryMap.Add(DialogueGUID);
	}

	History->VisitedNodeIndices.Add(NodeIndex);
}

bool FDlgMemory::IsNodeVisited(const FGuid& DialogueGUID, int32 NodeIndex) const
{
	const FDlgHistory* History = HistoryMap.Find(DialogueGUID);
	if (History == nullptr)
	{
		return false;
	}

	const int32* FoundIndex = History->VisitedNodeIndices.Find(NodeIndex);
	return FoundIndex != nullptr;
}
