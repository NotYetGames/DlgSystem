// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "Nodes/DlgNode.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgContextInternal.h"
#include "EngineUtils.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UObject interface
void UDlgNode::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.UE4Ver() >= VER_UE4_COOKED_ASSETS_IN_EDITOR_SUPPORT)
	{
		const FStripDataFlags StripFlags(Ar);
#if WITH_EDITOR
		if (!StripFlags.IsEditorDataStripped())
		{
			Ar << GraphNode;
		}
#endif
	}
#if WITH_EDITOR
	else
	{
		Ar << GraphNode;
	}
#endif
}

#if WITH_EDITOR
void UDlgNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Signal to the listeners
	OnDialogueNodePropertyChanged.Broadcast(PropertyChangedEvent, BroadcastPropertyEdgeIndexChanged);
	BroadcastPropertyEdgeIndexChanged = INDEX_NONE;
}

void UDlgNode::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	// The Super::PostEditChangeChainProperty will construct a new FPropertyChangedEvent that will only have the Property and the
	// MemberProperty name and it will call the PostEditChangeProperty, so we must get the array index of the Nodes modified from here.
	// If you want to preserve all the change history of the tree you must broadcast the event from here to the children, but be warned
	// that Property and MemberProperty are not set properly.
	BroadcastPropertyEdgeIndexChanged = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_STRING_CHECKED(UDlgNode, Children));
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UDlgNode::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	// Add the GraphNode to the referenced objects
	UDlgNode* This = CastChecked<UDlgNode>(InThis);
	Collector.AddReferencedObject(This->GraphNode, This);
	Super::AddReferencedObjects(InThis, Collector);
}
#endif //WITH_EDITOR
// End UObject interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own function
bool UDlgNode::HandleNodeEnter(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	check(DlgContext != nullptr);

	// Fire all the node enter events
	FireNodeEnterEvents(DlgContext);

	for (FDlgEdge& Edge : Children)
	{
		Edge.ConstructTextFromArguments(DlgContext, OwnerName);
	}

	return ReevaluateChildren(DlgContext, {});
}

void UDlgNode::FireNodeEnterEvents(UDlgContextInternal* DlgContext)
{
	for (const FDlgEvent& Event : EnterEvents)
	{
		UObject* Participant = DlgContext->GetParticipant(Event.ParticipantName);

		// Try parent
		if (!IsValid(Participant))
		{
			Participant = DlgContext->GetParticipant(OwnerName);
		}

		if (Participant == nullptr)
		{
			UE_LOG(LogDlgSystem, Error, TEXT("FireNodeEnterEvents: Dialogue = `%s`, NodeIndex = %d. Got non existent Participant Name, event call will fail!"), *GetDialogue()->GetPathName(), DlgContext->GetActiveNodeIndex());
		}

		Event.Call(Participant);
	}
}

void UDlgNode::UpdateTextNamespace(FText& Text)
{
	if (const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>())
	{
		if (Settings->DialogueTextLocalizationMode == EDlgTextLocalization::DlgIgnore)
		{
			return;
		}

		const auto Key = FTextInspector::GetKey(Text);
		if (Key.IsSet())
		{
			FString Namespace = Settings->DialogueTextNamespaceName;
			if (Settings->DialogueTextLocalizationMode == EDlgTextLocalization::DlgNamespacePerDialogue)
			{
				Namespace += GetOuter()->GetName();
			}
			Text = FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(*Text.ToString(), *Namespace, *Key.GetValue());
		}
	}
	
}

bool UDlgNode::ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyEvaluated)
{
	check(DlgContext != nullptr);

	TArray<const FDlgEdge*>& AvailableChildren = DlgContext->GetOptionArray();
	TArray<FDlgEdgeData>& AllChildren = DlgContext->GetAllOptionsArray();
	AvailableChildren.Empty();
	AllChildren.Empty();

	for (const FDlgEdge& Edge : Children)
	{
		const bool bSatisfied = Edge.Evaluate(DlgContext, { this });

		if (bSatisfied || Edge.bIncludeInAllOptionListIfUnsatisfied)
		{
			AllChildren.Add(FDlgEdgeData{ bSatisfied, &Edge });
		}
		if (bSatisfied)
		{
			AvailableChildren.Add(&Edge);
		}
	}

	// no child, but no end node?
	if (AvailableChildren.Num() == 0)
	{
		UE_LOG(LogDlgSystem, Warning, TEXT("Dialogue stucked: no valid child for a node!"));
		return false;
	}

	return true;
}

bool UDlgNode::CheckNodeEnterConditions(const UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	if (AlreadyVisitedNodes.Contains(this))
	{
		return true;
	}

	AlreadyVisitedNodes.Add(this);

	if (!FDlgCondition::EvaluateArray(EnterConditions, DlgContext, OwnerName))
	{
		return false;
	}

	if (!bCheckChildrenOnEvaluation)
	{
		return true;
	}

	// Has a valid child?
	return HasAnySatisfiedChild(DlgContext, AlreadyVisitedNodes);
}

bool UDlgNode::HasAnySatisfiedChild(const UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	for (const FDlgEdge& Edge : Children)
	{
		if (Edge.Evaluate(DlgContext, AlreadyVisitedNodes))
		{
			return true;
		}
	}

	return false;
}

bool UDlgNode::OptionSelected(int32 OptionIndex, UDlgContextInternal* DlgContext)
{
	TArray<const FDlgEdge*>& AvailableChildren = DlgContext->GetOptionArray();

	if (AvailableChildren.IsValidIndex(OptionIndex))
	{
		check(AvailableChildren[OptionIndex] != nullptr);
		return DlgContext->EnterNode(AvailableChildren[OptionIndex]->TargetIndex, {});
	}

	UE_LOG(LogDlgSystem,
		   Error,
		   TEXT("Failed to choose option index = %d - it only has %d valid options!"),
		   OptionIndex,
		   AvailableChildren.Num());

	return false;
}

const TArray<int32> UDlgNode::GetNodeOpenChildren_DEPRECATED() const
{
	TArray<int32> OutArray;
	const int32 EdgesNum = Children.Num();
	for (int32 EdgeIndex = 0; EdgeIndex < EdgesNum; EdgeIndex++)
	{
		if (!Children[EdgeIndex].IsValid())
		{
			OutArray.Add(EdgeIndex);
		}
	}

	return OutArray;
}

FDlgEdge* UDlgNode::GetMutableNodeChildForTargetIndex(int32 TargetIndex)
{
	for (FDlgEdge& Edge : Children)
	{
		if (Edge.TargetIndex == TargetIndex)
		{
			return &Edge;
		}
	}

	return nullptr;
}

void UDlgNode::GetAssociatedParticipants(TArray<FName>& OutArray) const
{
	if (OwnerName != NAME_None)
	{
		OutArray.AddUnique(OwnerName);
	}
}

UDlgDialogue* UDlgNode::GetDialogue() const
{
	return CastChecked<UDlgDialogue>(GetOuter());
}
// End own functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
