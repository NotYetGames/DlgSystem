// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "Nodes/DlgNode_Speech.h"

#include "DlgContextInternal.h"
#include "DlgSystemPrivatePCH.h"
#include "Logging/DlgLogger.h"
#include "DlgLocalizationHelper.h"

#if WITH_EDITOR
void UDlgNode_Speech::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const bool bTextChanged = PropertyName == GetMemberNameText();

	// rebuild text arguments
	if (bTextChanged || PropertyName == GetMemberNameTextArguments())
	{
		RebuildTextArguments(true);
	}
}

#endif

void UDlgNode_Speech::UpdateTextsValuesFromDefaultsAndRemappings(const UDlgSystemSettings* Settings, bool bEdges, bool bUpdateGraphNode)
{
	FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Text);
	Super::UpdateTextsValuesFromDefaultsAndRemappings(Settings, bEdges, bUpdateGraphNode);
}

void UDlgNode_Speech::UpdateTextsNamespacesAndKeys(const UDlgSystemSettings* Settings, bool bEdges, bool bUpdateGraphNode)
{
	FDlgLocalizationHelper::UpdateTextNamespaceAndKey(GetOuter(), Settings, Text);
	Super::UpdateTextsNamespacesAndKeys(Settings, bEdges, bUpdateGraphNode);
}

void UDlgNode_Speech::RebuildConstructedText(const UDlgContextInternal* DlgContext)
{
	if (TextArguments.Num() <= 0)
	{
		return;
	}

	FFormatNamedArguments OrderedArguments;
	for (const FDlgTextArgument& DlgArgument : TextArguments)
	{
		OrderedArguments.Add(DlgArgument.DisplayString, DlgArgument.ConstructFormatArgumentValue(DlgContext, OwnerName));
	}
	ConstructedText = FText::AsCultureInvariant(FText::Format(Text, OrderedArguments));
}

bool UDlgNode_Speech::HandleNodeEnter(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	RebuildConstructedText(DlgContext);
	return Super::HandleNodeEnter(DlgContext, NodesEnteredWithThisStep);
}

bool UDlgNode_Speech::ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyEvaluated)
{
	if (bIsVirtualParent)
	{
		check(DlgContext != nullptr);
		DlgContext->GetOptionArray().Empty();
		DlgContext->GetAllOptionsArray().Empty();

		// stop endless loop
		if (AlreadyEvaluated.Contains(this))
		{
			FDlgLogger::Get().Warning(
				TEXT("Endless loop detected in ReevaluateChildren call: a virtual parent became his own parent!"
							"This is not supposed to happen, the dialogue is terminated!")
			);
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


void UDlgNode_Speech::GetAssociatedParticipants(TArray<FName>& OutArray) const
{
	Super::GetAssociatedParticipants(OutArray);
	for (const FDlgTextArgument& TextArgument : TextArguments)
	{
		if (TextArgument.ParticipantName != NAME_None)
		{
			OutArray.AddUnique(TextArgument.ParticipantName);
		}
	}
}
