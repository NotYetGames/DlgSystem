// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "Nodes/DlgNode.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

#include "DlgContext.h"
#include "Logging/DlgLogger.h"
#include "DlgLocalizationHelper.h"
#include "DlgDialogueParticipant.h"
#include "Sound/SoundWave.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UObject interface
void UDlgNode::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.UE4Ver() >= VER_UE4_COOKED_ASSETS_IN_EDITOR_SUPPORT)
	{
		// NOTE: This modifies the Archive
		// DO NOT REMOVE THIS
		const FStripDataFlags StripFlags(Ar);

		// Only in editor, add the graph node
#if WITH_EDITOR
		if (!StripFlags.IsEditorDataStripped())
		{
			Ar << GraphNode;
		}
#endif // WITH_EDITOR
	}
	else
	{
		// Super old version, is this possible?
#if WITH_EDITOR
		Ar << GraphNode;
#endif // WITH_EDITOR
	}
}

void UDlgNode::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UDlgNode* This = CastChecked<UDlgNode>(InThis);

	// Add the GraphNode to the referenced objects
#if WITH_EDITOR
	Collector.AddReferencedObject(This->GraphNode, This);
#endif

	Super::AddReferencedObjects(InThis, Collector);
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


#endif //WITH_EDITOR
// End UObject interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own function
bool UDlgNode::HandleNodeEnter(UDlgContext& Context, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	// Fire all the node enter events
	FireNodeEnterEvents(Context);

	for (FDlgEdge& Edge : Children)
	{
		Edge.RebuildConstructedText(Context, OwnerName);
	}

	return ReevaluateChildren(Context, {});
}

void UDlgNode::FireNodeEnterEvents(UDlgContext& Context)
{
	for (const FDlgEvent& Event : EnterEvents)
	{
		// Get Participant from either event or parent
		UObject* Participant = Context.GetMutableParticipant(Event.ParticipantName);
		if (!IsValid(Participant))
		{
			Participant = Context.GetMutableParticipant(OwnerName);
		}

		if (Participant == nullptr)
		{
			FDlgLogger::Get().Warningf(
				TEXT("FireNodeEnterEvents - Got non existent Participant Name (INVALID Participant) event call will FAIL.\nContext:\n\t%s"),
				*GetDialogue()->GetPathName(), Context.GetActiveNodeIndex(), *Context.GetContextString()
			);
		}

		Event.Call(Context, Participant);
	}
}

bool UDlgNode::ReevaluateChildren(UDlgContext& Context, TSet<const UDlgNode*> AlreadyEvaluated)
{
	TArray<FDlgEdge>& AvailableChildren = Context.GetMutableOptionsArray();
	TArray<FDlgEdgeData>& AllChildren = Context.GetAllMutableOptionsArray();
	AvailableChildren.Empty();
	AllChildren.Empty();

	for (const FDlgEdge& Edge : Children)
	{
		const bool bSatisfied = Edge.Evaluate(Context, { this });

		if (bSatisfied || Edge.bIncludeInAllOptionListIfUnsatisfied)
		{
			AllChildren.Add(FDlgEdgeData{ bSatisfied, Edge });
		}
		if (bSatisfied)
		{
			AvailableChildren.Add(Edge);
		}
	}

	// no child, but no end node?
	if (AvailableChildren.Num() == 0)
	{
		FDlgLogger::Get().Errorf(
			TEXT("ReevaluateChildren - no valid child for a NODE.\nContext:\n\t%s"),
			*Context.GetContextString()
		);
		return false;
	}

	return true;
}

bool UDlgNode::CheckNodeEnterConditions(const UDlgContext& Context, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	if (AlreadyVisitedNodes.Contains(this))
	{
		return true;
	}

	AlreadyVisitedNodes.Add(this);
	if (!FDlgCondition::EvaluateArray(Context, EnterConditions, OwnerName))
	{
		return false;
	}
	if (!bCheckChildrenOnEvaluation)
	{
		return true;
	}

	// Has a valid child?
	return HasAnySatisfiedChild(Context, AlreadyVisitedNodes);
}

bool UDlgNode::HasAnySatisfiedChild(const UDlgContext& Context, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	for (const FDlgEdge& Edge : Children)
	{
		if (Edge.Evaluate(Context, AlreadyVisitedNodes))
		{
			return true;
		}
	}

	return false;
}

bool UDlgNode::OptionSelected(int32 OptionIndex, UDlgContext& Context)
{
	const TArray<FDlgEdge>& AvailableChildren = Context.GetOptionsArray();
	if (AvailableChildren.IsValidIndex(OptionIndex))
	{
		check(AvailableChildren[OptionIndex].IsValid());
		return Context.EnterNode(AvailableChildren[OptionIndex].TargetIndex, {});
	}

	FDlgLogger::Get().Errorf(
		TEXT("OptionSelected - Failed to choose OptionIndex = %d - it only has %d valid options.\nContext:\n\t%s"),
		OptionIndex, AvailableChildren.Num(), *Context.GetContextString()
	);
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


void UDlgNode::UpdateTextsValuesFromDefaultsAndRemappings(
	const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode
)
{
	// We only care about edges here
	if (bEdges)
	{
		const bool bSkipAfterFirstChild = Settings.bSetDefaultEdgeTextOnFirstChildOnly;
		if (Settings.bSetDefaultEdgeTexts)
		{
			const UDlgDialogue* Dialogue = GetDialogue();
			for (FDlgEdge& Edge : Children)
			{
				Edge.UpdateTextValueFromDefaultAndRemapping(*Dialogue, *this, Settings, false);

				// Set only one, kill the rest
				if (bSkipAfterFirstChild)
				{
					break;
				}
			}
		}

		// Update the rest of the texts remapping
		for (FDlgEdge& Edge : Children)
		{
			FDlgLocalizationHelper::UpdateTextFromRemapping(Settings, Edge.GetMutableUnformattedText());
		}
	}

	if (bUpdateGraphNode)
	{
		UpdateGraphNode();
	}
}

void UDlgNode::UpdateTextsNamespacesAndKeys(const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode)
{
	if (bEdges)
	{
		UObject* Outer = GetOuter();
		for (FDlgEdge& Edge : Children)
		{
			Edge.UpdateTextsNamespacesAndKeys(Outer, Settings);
		}
	}

	if (bUpdateGraphNode)
	{
		UpdateGraphNode();
	}
}

void UDlgNode::RebuildTextArguments(bool bEdges, bool bUpdateGraphNode)
{
	if (bEdges)
	{
		for (FDlgEdge& Edge : Children)
		{
			Edge.RebuildTextArguments();
		}
	}

	if (bUpdateGraphNode)
	{
		UpdateGraphNode();
	}
}

void UDlgNode::UpdateGraphNode()
{
#if WITH_EDITOR
	UDlgDialogue::GetDialogueEditorAccess()->UpdateGraphNodeEdges(GraphNode);
#endif // WITH_EDITOR
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

USoundWave* UDlgNode::GetNodeVoiceSoundWave() const
{
	return Cast<USoundWave>(GetNodeVoiceSoundBase());
}

// End own functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
