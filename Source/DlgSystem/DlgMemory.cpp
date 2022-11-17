// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgMemory.h"
#include "DlgHelper.h"

void FDlgHistory::Add(int32 NodeIndex, const FGuid& NodeGUID)
{
	if (NodeIndex >= 0)
	{
		VisitedNodeIndices.Add(NodeIndex);
	}
	if (NodeGUID.IsValid())
	{
		VisitedNodeGUIDs.Add(NodeGUID);
	}
}

bool FDlgHistory::Contains(int32 NodeIndex, const FGuid& NodeGUID) const
{
	// Use GUID
	if (CanUseGUIDForSearch() && NodeGUID.IsValid())
	{
		return VisitedNodeGUIDs.Contains(NodeGUID);
	}

	// FallBack to Node Index
	return VisitedNodeIndices.Contains(NodeIndex);
}

bool FDlgHistory::operator==(const FDlgHistory& Other) const
{
	return FDlgHelper::IsSetEqual(VisitedNodeIndices, Other.VisitedNodeIndices)
		&& FDlgHelper::IsSetEqual(VisitedNodeGUIDs, Other.VisitedNodeGUIDs);
}

FDlgNodeSavedData& FDlgHistory::GetNodeData(const FGuid& NodeGUID)
{
	return NodeData.FindOrAdd(NodeGUID);
}

