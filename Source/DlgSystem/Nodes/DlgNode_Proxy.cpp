// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgNode_Proxy.h"

#include "DlgSystem/DlgContext.h"
#include "DlgSystem/Logging/DlgLogger.h"

bool UDlgNode_Proxy::HandleNodeEnter(UDlgContext& Context, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	FireNodeEnterEvents(Context);

	if (NodesEnteredWithThisStep.Contains(this))
	{
		FDlgLogger::Get().Errorf(
			TEXT("ProxyNode::HandleNodeEnter - Failed to enter proxy node, it was entered multiple times in a single step."
					"Theoretically with some condition magic it could make sense, but chances are that it is an endless loop,"
					"thus entering the same proxy twice with a single step is not supported. Dialogue is terminated.\nContext:\n\t%s"),
			*Context.GetContextString()
		);

		return false;
	}
	NodesEnteredWithThisStep.Add(this);

	return Context.EnterNode(NodeIndex, NodesEnteredWithThisStep);
}

bool UDlgNode_Proxy::CheckNodeEnterConditions(const UDlgContext& Context, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	if (!Super::CheckNodeEnterConditions(Context, AlreadyVisitedNodes))
	{
		return false;
	}

	const UDlgNode* Node = Context.GetNodeFromIndex(NodeIndex);
	check(Node);
	return Node->CheckNodeEnterConditions(Context, AlreadyVisitedNodes);
}

void UDlgNode_Proxy::RemapOldIndicesWithNew(const TMap<int32, int32>& OldToNewIndexMap)
{
	if (const int32* NewIndexPtr = OldToNewIndexMap.Find(NodeIndex))
	{
		NodeIndex = *NewIndexPtr;
	}
}
