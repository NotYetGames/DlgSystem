// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEdge.h"

#include "DlgConstants.h"
#include "DlgContext.h"
#include "DlgLocalizationHelper.h"
#include "Nodes/DlgNode_Selector.h"
#include "Nodes/DlgNode_Speech.h"

bool FDlgEdge::IsTextVisible(const UDlgNode& ParentNode)
{
	// Selector node
	if (ParentNode.IsA<UDlgNode_Selector>())
	{
		return false;
	}

	// Virtual parent node
	if (const UDlgNode_Speech* Node = Cast<UDlgNode_Speech>(&ParentNode))
	{
		if (Node->IsVirtualParent())
		{
			return false;
		}
	}

	return true;
}

void FDlgEdge::UpdateTextValueFromDefaultAndRemapping(
	const UDlgDialogue& ParentDialogue,
	const UDlgNode& ParentNode,
	const UDlgSystemSettings& Settings,
	bool bUpdateFromRemapping
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

	if (Settings.bSetDefaultEdgeTexts)
	{
		// Only if empty
		if (GetUnformattedText().IsEmpty())
		{
			if (ParentDialogue.IsEndNode(TargetIndex))
			{
				// End Node
				SetText(Settings.DefaultTextEdgeToEndNode);
			}
			else
			{
				// Normal node
				SetText(Settings.DefaultTextEdgeToNormalNode);
			}
		}
	}

	// Update text remapping
	if (bUpdateFromRemapping)
	{
		FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Text);
	}
}

void FDlgEdge::UpdateTextsNamespacesAndKeys(const UObject* ParentObject, const UDlgSystemSettings& Settings)
{
	if (!IsValid())
	{
		return;
	}

	FDlgLocalizationHelper::UpdateTextNamespaceAndKey(ParentObject, Settings, Text);
}

bool FDlgEdge::Evaluate(const UDlgContext& Context, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	if (!IsValid())
	{
		return false;
	}

	// Check target node enter conditions
	if (!Context.IsNodeEnterable(TargetIndex, AlreadyVisitedNodes))
	{
		return false;
	}

	// Check this edge conditions
	return FDlgCondition::EvaluateArray(Context, Conditions);
}

void FDlgEdge::RebuildConstructedText(const UDlgContext& Context, FName FallbackParticipantName)
{
	if (TextArguments.Num() <= 0)
	{
		return;
	}

	FFormatNamedArguments OrderedArguments;
	for (const FDlgTextArgument& DlgArgument : TextArguments)
	{
		OrderedArguments.Add(DlgArgument.DisplayString, DlgArgument.ConstructFormatArgumentValue(Context, FallbackParticipantName));
	}
	ConstructedText = FText::AsCultureInvariant(FText::Format(Text, OrderedArguments));
}
