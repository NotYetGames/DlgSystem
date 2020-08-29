// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgMemory.h"
#include "DlgHelper.h"

bool FDlgHistory::operator==(const FDlgHistory& Other) const
{
	return FDlgHelper::IsSetEqual(VisitedNodeIndices, Other.VisitedNodeIndices)
		&& FDlgHelper::IsSetEqual(VisitedNodeGUIDs, Other.VisitedNodeGUIDs);
}
