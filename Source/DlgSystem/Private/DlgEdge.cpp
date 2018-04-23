// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgEdge.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgContextInternal.h"

const FDlgEdge& FDlgEdge::GetInvalidEdge()
{
	static FDlgEdge DlgEdge;
	return DlgEdge;
}

bool FDlgEdge::Evaluate(const UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	if (!IsValid())
	{
		return false;
	}

	// Check target node enter conditions
	if (!DlgContext->IsNodeEnterable(TargetIndex, AlreadyVisitedNodes))
	{
		return false;
	}

	// Check this edge conditions
	return FDlgCondition::EvaluateArray(Conditions, DlgContext);
}

void FDlgEdge::ConstructTextFromArguments(const UDlgContextInternal* DlgContext, FName NodeOwnerName)
{
	if (TextArguments.Num() > 0)
	{
		FFormatNamedArguments OrderedArguments;

		for (const FDlgTextArgument& DlgArgument : TextArguments)
		{
			OrderedArguments.Add(DlgArgument.DisplayString, DlgArgument.ConstructFormatArgumentValue(DlgContext, NodeOwnerName));
		}

		ConstructedText = FText::Format(Text, OrderedArguments);
	}
}


FArchive& operator<<(FArchive &Ar, FDlgEdge& DlgEdge)
{
	Ar << DlgEdge.TargetIndex;
	Ar << DlgEdge.Text;
	Ar << DlgEdge.Conditions;
	Ar << DlgEdge.SpeakerState;
	Ar << DlgEdge.bIncludeInAllOptionListIfUnsatisfied;
	return Ar;
}
