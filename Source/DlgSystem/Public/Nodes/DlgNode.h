// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreTypes.h"
#if WITH_EDITOR
#include "EdGraph/EdGraphNode.h"
#endif

#include "DlgEdge.h"
#include "DlgCondition.h"
#include "DlgEvent.h"

#include "DlgNode.generated.h"


class UDlgContextInternal;
class UDlgNode;
class USoundWave;
class UDialogueWave;
struct FDlgTextArgument;

/**
 *  Abstract base class for Dialogue nodes
 *  Depending on the implementation in the child class the dialogue node can contain one or more lines of one or more participants,
 *  or simply some logic to go on in the UDlgNode graph
 */
UCLASS(BlueprintType, Abstract, EditInlineNew)
class DLGSYSTEM_API UDlgNode : public UObject
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.
	/** UObject serializer. */
	void Serialize(FArchive& Ar) override;

	/** @return a one line description of an object. */
	FString GetDesc() override { return TEXT("INVALID DESCRIPTION"); }

#if WITH_EDITOR
	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the UStructProperty member variable that contains the property that was modified.
	 */
	void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param InThis Object to collect references from.
	 * @param Collector	FReferenceCollector objects to be used to collect references.
	*/
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	// End UObject Interface.

	// Begin own function
	// Used internally by the Dialogue editor:
	virtual FString GetNodeTypeString() const { return TEXT("INVALID"); }
#endif //WITH_EDITOR

#if WITH_EDITOR
	void SetGraphNode(UEdGraphNode* InNode) { GraphNode = InNode; }
	void ClearGraphNode() { GraphNode = nullptr; }
	UEdGraphNode* GetGraphNode() const { return GraphNode; }
#endif

	/** Broadcasts whenever a property of this dialogue changes. */
	DECLARE_EVENT_TwoParams(UDlgNode, FDialogueNodePropertyChanged, const FPropertyChangedEvent& /* PropertyChangedEvent */, int32 /* EdgeIndexChanged */);
	FDialogueNodePropertyChanged OnDialogueNodePropertyChanged;

	virtual bool HandleNodeEnter(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> NodesEnteredWithThisStep);

	virtual bool ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyEvaluated);

	virtual bool CheckNodeEnterConditions(const UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyVisitedNodes) const;
	virtual bool HasAnySatisfiedChild(const UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyVisitedNodes) const;

	virtual bool OptionSelected(int32 OptionIndex, UDlgContextInternal* DlgContext);

	// Getters/Setters:
	// For the ParticipantName
	virtual FName GetNodeParticipantName() const { return OwnerName; }
	virtual void SetNodeParticipantName(const FName& InName) { OwnerName = InName; }

	// For the EnterConditions
	virtual const TArray<FDlgCondition>& GetNodeEnterConditions() const { return EnterConditions; }
	virtual void SetNodeEnterConditions(const TArray<FDlgCondition>& InEnterConditions) { EnterConditions = InEnterConditions; }

	/** Gets the mutable enter condition at location EnterConditionIndex. */
	virtual FDlgCondition* GetMutableEnterConditionAt(int32 EnterConditionIndex)
	{
		check(EnterConditions.IsValidIndex(EnterConditionIndex));
		return &EnterConditions[EnterConditionIndex];
	}

	// For the EnterEvents
	virtual const TArray<FDlgEvent>& GetNodeEnterEvents() const { return EnterEvents; }
	virtual void SetNodeEnterEvents(const TArray<FDlgEvent>& InEnterEvents) { EnterEvents = InEnterEvents; }

	// For the Children
	/** Gets this nodes children (edges) as a const/mutable array */
	virtual const TArray<FDlgEdge>& GetNodeChildren() const { return Children; }
	virtual void SetNodeChildren(const TArray<FDlgEdge>& InChildren) { Children = InChildren; }

	/** Adds an Edge to the end of the Children Array. */
	virtual void AddNodeChild(const FDlgEdge& InChild) { Children.Add(InChild); }

	/** Removes the Edge at the specified EdgeIndex location. */
	virtual void RemoveChildAt(int32 EdgeIndex)
	{
		check(Children.IsValidIndex(EdgeIndex));
		Children.RemoveAt(EdgeIndex);
	}

	/** Removes all edges/children */
	virtual void RemoveAllChildren() { Children.Empty(); }

	/** Gets the mutable edge/child at location EdgeIndex. */
	virtual FDlgEdge* GetSafeMutableNodeChildAt(int32 EdgeIndex)
	{
		check(Children.IsValidIndex(EdgeIndex));
		return &Children[EdgeIndex];
	}

	/** Unsafe version, can be null */
	virtual FDlgEdge* GetMutableNodeChildAt(int32 EdgeIndex)
	{
		return Children.IsValidIndex(EdgeIndex) ? &Children[EdgeIndex] : nullptr; 
	}

	/** Gets the mutable Edge that corresponds to the provided TargetIndex or nullptr if nothing was found. */
	virtual FDlgEdge* GetMutableNodeChildForTargetIndex(int32 TargetIndex);

	/** Gets all the edges (children) indicies that DO NOT have a valid TargetIndex (is negative). */
	const TArray<int32> GetNodeOpenChildren_DEPRECATED() const;

	/** Gathers associated participants, they are only added to the array if they are not yet there */
	virtual void GetAssociatedParticipants(TArray<FName>& OutArray) const;

	/** Gets the text arguments for this Node (if any). Used for FText::Format */
	virtual const TArray<FDlgTextArgument>& GetTextArguments() const
	{
		static TArray<FDlgTextArgument> EmptyArray;
		return EmptyArray;
	};

	/** Gets the Text of this Node. This can be the final formatted string. */
	virtual const FText& GetNodeText() const { return FText::GetEmpty(); }

	/**
	 * Gets the Raw unformatted Text of this Node. Usually the same as GetNodeText but in case the node supports formatted string this
	 * is the raw form with all the arguments intact. To get the text arguments call GetTextArguments.
	 */
	virtual const FText& GetNodeUnformattedText() const { return GetNodeText(); }

	/** Gets the voice of this Node as a SoundWave. */
	virtual USoundWave* GetNodeVoiceSoundWave() const { return nullptr; }

	/** Gets the voice of this Node as a DialogueWave. Only the first Dialogue context in the wave should be used. */
	virtual UDialogueWave* GetNodeVoiceDialogueWave() const { return nullptr; }

	/** Gets the speaker state ordered to this node (can be used e.g. for icon selection) */
	virtual FName GetSpeakerState() const { return NAME_None; }
	virtual void AddSpeakerStates(TSet<FName>& States) const {};

	/** Helper method to get directly the Dialogue */
	class UDlgDialogue* GetDialogue() const;

	/** Helper functions to get the names of some properties. Used by the DlgSystemEditor module. */
	static FName GetMemberNameOwnerName() { return GET_MEMBER_NAME_CHECKED(UDlgNode, OwnerName); }
	static FName GetMemberNameCheckChildrenOnEvaluation() { return GET_MEMBER_NAME_CHECKED(UDlgNode, bCheckChildrenOnEvaluation); }
	static FName GetMemberNameEnterConditions() { return GET_MEMBER_NAME_CHECKED(UDlgNode, EnterConditions); }
	static FName GetMemberNameEnterEvents() { return GET_MEMBER_NAME_CHECKED(UDlgNode, EnterEvents); }
	static FName GetMemberNameChildren() { return GET_MEMBER_NAME_CHECKED(UDlgNode, Children); }

protected:

	void FireNodeEnterEvents(UDlgContextInternal* DlgContext);

protected:

#if WITH_EDITORONLY_DATA
	/** Node's Graph representation, used to get position. */
	UPROPERTY(Meta = (DlgNoExport))
	UEdGraphNode* GraphNode = nullptr;

	// Used to build the change event and broadcast it
	int32 BroadcastPropertyEdgeIndexChanged = INDEX_NONE;
#endif

	/** Name of a participant (speaker) associated with this node. */
	UPROPERTY(EditAnywhere, Category = DialogueNodeData, Meta = (DisplayName = "Participant Name"))
	FName OwnerName;

	/**
	 *  If it is set the node is only satisfied if at least one of its children is
	 *  Should not be used if entering this node can modify the condition results of its children.
	 */
	UPROPERTY(EditAnywhere, Category = DialogueNodeData)
	bool bCheckChildrenOnEvaluation = false;

	/** Conditions necessary to enter this node */
	UPROPERTY(EditAnywhere, Category = DialogueNodeData)
	TArray<FDlgCondition> EnterConditions;

	/** Events fired when the node is reached in the dialogue */
	UPROPERTY(EditAnywhere, Category = DialogueNodeData)
	TArray<FDlgEvent> EnterEvents;

	/** Edges that point to Children of this Node */
	UPROPERTY(EditAnywhere, EditFixedSize, AdvancedDisplay, Category = DialogueNodeData)
	TArray<FDlgEdge> Children;
};
