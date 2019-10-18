// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgEdge.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgContextInternal.h"
#include "DlgLocalizationHelper.h"

const FDlgEdge& FDlgEdge::GetInvalidEdge()
{
	static FDlgEdge DlgEdge;
	return DlgEdge;
}

void FDlgEdge::UpdateDefaultTexts(const UDlgDialogue* ParentDialogue, const UDlgSystemSettings* Settings)
{
	if (!Settings->bSetDefaultEdgeTexts)
	{
		return;
	}
	if (!IsValid())
	{
		return;
	}

	// Only if empty
	if (GetUnformattedText().IsEmpty())
	{
		if (ParentDialogue->IsEndNode(TargetIndex))
		{
			// End Node
			SetText(Settings->DefaultTextEdgeToEndNode);
		}
		else
		{
			// Normal node
			SetText(Settings->DefaultTextEdgeToNormalNode);
		}
	}
}

void FDlgEdge::UpdateTextsNamespacesAndKeys(const UObject* ParentObject, const UDlgSystemSettings* Settings)
{
	if (!IsValid())
	{
		return;
	}
	
	FDlgLocalizationHelper::UpdateTextNamespace(ParentObject, Settings, Text);
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

void FDlgEdge::RebuildConstructedText(const UDlgContextInternal* DlgContext, FName NodeOwnerName)
{
	if (TextArguments.Num() <= 0)
	{
		return;
	}
	
	FFormatNamedArguments OrderedArguments;
	for (const FDlgTextArgument& DlgArgument : TextArguments)
	{
		OrderedArguments.Add(DlgArgument.DisplayString, DlgArgument.ConstructFormatArgumentValue(DlgContext, NodeOwnerName));
	}
	ConstructedText = FText::AsCultureInvariant(FText::Format(Text, OrderedArguments));
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
