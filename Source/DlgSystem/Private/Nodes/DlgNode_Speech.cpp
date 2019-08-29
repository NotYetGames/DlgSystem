// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "Nodes/DlgNode_Speech.h"

#include "DlgContextInternal.h"
#include "DlgSystemPrivatePCH.h"

#if WITH_EDITOR
void UDlgNode_Speech::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const bool bTextChanged = PropertyName == UDlgNode_Speech::GetMemberNameText();
	// rebuild text arguments
	if (bTextChanged || PropertyName == UDlgNode_Speech::GetMemberNameTextArguments())
	{
		RebuildTextArguments();

		if (bTextChanged)
		{
			// TODO: maybe store key in preeditchangeproperty to preserve it?!
			UpdateTextNamespace(Text);
		}
	}
}

#endif

void UDlgNode_Speech::RebuildTextArguments()
{
	FDlgTextArgument::UpdateTextArgumentArray(Text, TextArguments);
	ConstructedText = Text;
}

bool UDlgNode_Speech::HandleNodeEnter(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	if (TextArguments.Num() > 0)
	{
		FFormatNamedArguments OrderedArguments;

		for (const FDlgTextArgument& DlgArgument : TextArguments)
		{
			OrderedArguments.Add(DlgArgument.DisplayString, DlgArgument.ConstructFormatArgumentValue(DlgContext, OwnerName));
		}

		ConstructedText = FText::Format(Text, OrderedArguments);
	}

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
