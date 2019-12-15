// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgEdge.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgContextInternal.h"
#include "DlgLocalizationHelper.h"
#include "Nodes/DlgNode_Selector.h"
#include "Nodes/DlgNode_Speech.h"

const FDlgEdge& FDlgEdge::GetInvalidEdge()
{
	static FDlgEdge DlgEdge;
	return DlgEdge;
}

bool FDlgEdge::IsTextVisible(const UDlgNode* ParentNode)
{
	if (!::IsValid(ParentNode))
	{
		return false;
	}

	// Selector node
	if (ParentNode->IsA<UDlgNode_Selector>())
	{
		return false;
	}

	// Virtual parent node
	if (const UDlgNode_Speech* Node = Cast<UDlgNode_Speech>(ParentNode))
	{
		if (Node->IsVirtualParent())
		{
			return false;
		}
	}
	
	return true; 
}

void FDlgEdge::UpdateTextValueFromDefaultAndRemapping(
	const UDlgDialogue* ParentDialogue, const UDlgNode* ParentNode, const UDlgSystemSettings* Settings, bool bUpdateFromRemapping
)
{
	if (!IsValid())
	{
		return;
	}

	// Clear the current text as it won't be visible anyways
	if (!IsTextVisible(ParentNode))
	{
		SetText(FText::GetEmpty());
		return;
	}
	
	if (Settings->bSetDefaultEdgeTexts)
	{
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

	// Update text remapping
	if (bUpdateFromRemapping)
	{
		FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Text);
	}
}

void FDlgEdge::UpdateTextsNamespacesAndKeys(const UObject* ParentObject, const UDlgSystemSettings* Settings)
{
	if (!IsValid())
	{
		return;
	}
	
	FDlgLocalizationHelper::UpdateTextNamespaceAndKey(ParentObject, Settings, Text);
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
