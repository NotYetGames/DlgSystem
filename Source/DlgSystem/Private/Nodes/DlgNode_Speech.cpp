// Fill out your copyright notice in the Description page of Project Settings.
#include "DlgNode_Speech.h"
#include "DlgContextInternal.h"


bool UDlgNode_Speech::ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<UDlgNode*> AlreadyEvaluated)
{
	if (bIsVirtualParent)
	{
		check(DlgContext != nullptr);
		DlgContext->GetOptionArray().Empty();

		// stop endless loop
		if (AlreadyEvaluated.Contains(this))
		{
			UE_LOG(LogDlgSystem, Warning, TEXT("Endless loop detected in ReevaluateChildren call: a virtual parent became his own parent!"
												"This is not supposed to happen, the dialogue is terminated!"));
			return false;
		}

		AlreadyEvaluated.Add(this);

		for (const FDlgEdge& Edge : Children)
		{
			// Find first satisfied child
			if (Edge.Evaluate(DlgContext, { this }))
			{
				UDlgNode* Node = DlgContext->GetNode(Edge.TargetIndex);
				if (Node != nullptr)
				{
					// Get Grandchildren
					return Node->ReevaluateChildren(DlgContext, AlreadyEvaluated);
				}
			}
		}
		return false;
	}

	// Normal speech node
	return Super::ReevaluateChildren(DlgContext, AlreadyEvaluated);
}
