// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgDialogue.h"
#include "Nodes/DlgNode.h"
#include "DlgContext.generated.h"

class USoundWave;
class UDialogueWave;
class UTexture2D;
class UDlgNodeData;
class UDlgNode;

/** Used to store temporary state of edges */
struct FDlgEdgeData
{
	bool bSatisfied = false;
	const FDlgEdge* EdgePtr = nullptr;

	FDlgEdgeData(bool bSat, const FDlgEdge* Ptr) : bSatisfied{ bSat }, EdgePtr{ Ptr } {};
};


/**
 *  Class representing an active dialogue, can be used to gain information and to control it
 *  Should be controlled from Player Character/Player controller
 *  For starting a dialogue check UDlgManager - the proper function creates an UDlgContext for you
 *
 *  Call ChooseChild() if an option is selected
 *  If the return value is false the dialogue is over and the context should be dropped
 *  This abstract class contains the outer functionality only
 */
UCLASS(BlueprintType, Abstract)
class DLGSYSTEM_API UDlgContext : public UObject
{
	GENERATED_BODY()
public:

	/**
	 * Chooses the option with index OptionIndex of the active node index and it enters that node.
	 * Typically called based on user input.
	 * NOTICE: If the return value is false the dialogue is over and the context should be dropped
	 *
	 * @return true if the dialogue did not end, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = DialogueControl)
	virtual bool ChooseChild(int32 OptionIndex) { return false; }

	/**
	 *  Exactly as above but expects an index from the AllOptions array
	 *  If the index is invalid or the selected edge is not satisfied the call fails
	 */
	UFUNCTION(BlueprintCallable, Category = DialogueControl)
	bool ChooseChildBasedOnAllOptionIndex(int32 Index);

	/**
	 * Normally the children of the active node are checked only once, when the conversation enters the node.
	 * If an option can appear/disappear real time in the middle of the conversation this function should be called manually each frame
	 */
	UFUNCTION(BlueprintCallable, Category = DialogueControl)
	virtual void ReevaluateChildren() {}

	/** Use these functions if you don't care about unsatisfied player options: */

	/** Gets the number of children with satisfied conditions (number of options) */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	int32 GetOptionNum() const { return AvailableChildren.Num(); }

	/** Gets the Text of the (satisfied) option with index OptionIndex  */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	const FText& GetOptionText(int32 OptionIndex) const;

	/** Gets the SpeakerState of the (satisfied) edge with index OptionIndex */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	FName GetOptionSpeakerState(int32 OptionIndex) const;

	/** Gets the edge representing a player option from the satisfied options */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	const FDlgEdge& GetOption(int32 OptionIndex) const;


	/**
	 *  Use these functions bellow if you don't care about unsatisfied player options:
	 *  DO NOT missuse the indices above and bellow! The functions above expect < GetOptionNum(), bellow < GetAllOptionNum()
	 */

	/** Gets the number of children (both satisfied and unsatisfied ones are counted) */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	int32 GetAllOptionNum() const { return AllChildren.Num(); }

	/** Gets the Text of an option from the all list, which includes the unsatisfied ones as well */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	const FText& GetOptionTextFromAll(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = DialogueData)
	bool IsOptionSatisfied(int32 Index) const;

	/** Gets the SpeakerState of the edge with index OptionIndex */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	FName GetOptionSpeakerStateFromAll(int32 Index) const;

	/** Gets the edge representing a player option from all options */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	const FDlgEdge& GetOptionFromAll(int32 Index) const;


	/** Gets the Text of the active node index */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	const FText& GetActiveNodeText() const;

	/** Gets the SpeakerState of the active node index */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	FName GetActiveSpeakerState() const;

	/** Gets the Voice as a Sound Wave of the active node index */
	UFUNCTION(BlueprintPure, Category = DialogueDataSound)
	USoundWave* GetActiveNodeVoiceSoundWave() const;

	/** Gets the Voice as a Dialogue Wave of the active node index */
	UFUNCTION(BlueprintPure, Category = DialogueDataSound)
	UDialogueWave* GetActiveNodeVoiceDialogueWave() const;

	UFUNCTION(BlueprintPure, Category = DialogueData)
	UObject* GetActiveNodeGenericData() const;

	UFUNCTION(BlueprintPure, Category = DialogueData)
	UDlgNodeData* GetActiveNodeData() const;

	/** Gets the Icon associated with the active node participant name (owner name). */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	UTexture2D* GetActiveParticipantIcon() const;

	/** Gets the Object associated with the active node participant name (owner name). */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	UObject* GetActiveParticipant() const;

	/** Gets the active node participant name (owner name). */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	FName GetActiveParticipantName() const;

	UFUNCTION(BlueprintPure, Category = DialogueData)
	UObject* GetParticipant(FName DlgParticipantName)
	{
		return const_cast<UObject*>(GetConstParticipant(DlgParticipantName));
	}

	const UObject* GetConstParticipant(FName DlgParticipantName) const;
	const TMap<FName, UObject*>& GetParticipantMap() const { return Participants; }

	UFUNCTION(BlueprintPure, Category = DialogueData)
	int32 GetActiveNodeIndex() const { return ActiveNodeIndex; }

	/** Returns the indices which were visited inside this single context. For global data check DlgMemory */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	const TSet<int32>& GetVisitedNodeIndices() const { return VisitedNodeIndices; }

	/**
	 *  Checks if the node connected directly to one of the active player choices was already visited or not
	 *  Does not handle complicated logic - if the said node is a logical one it will still check that node, and not one
	 *  of its children
	 *
	 * @param Index  Index of the edge/player choice to test
	 * @param bLocalHistory If true, only the history of this dialogue context is checked. If false, it is a global check
	 * @param bIndexSkipsUnsatisfiedEdges  Decides if the index is in the [0, GetOptionNum()[ interval (if true), or in the [0, GetAllOptionNum()[ (if false)
	 * @return true if the node was already visited
	 */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	bool IsEdgeConnectedToVisitedNode(int32 Index, bool bLocalHistory = false, bool bIndexSkipsUnsatisfiedEdges = true) const;


	/**
	 *  Checks if the node is connected directly to an end node or not
	 *  Does not handle complicated logic - if the said node is a logical one it will still check that node, and not one
	 *  of its children
	 *
	 * @param Index  Index of the edge/player choice to test
	 * @param bIndexSkipsUnsatisfiedEdges  Decides if the index is in the [0, GetOptionNum()[ interval (if true), or in the [0, GetAllOptionNum()[ (if false)
	 * @return true if the node is an end node
	 */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	bool IsEdgeConnectedToEndNode(int32 Index, bool bIndexSkipsUnsatisfiedEdges = true) const;

	// Helper methods to get some Dialogue properties
	FName GetDialogueName() const { check(Dialogue); return Dialogue->GetDlgFName(); }
	FGuid GetDialogueGuid() const { check(Dialogue); return Dialogue->GetDlgGuid(); }
	FString GetDialoguePathName() const { check(Dialogue); return Dialogue->GetPathName(); }

protected:
	// Methods implemented by UDlgContextInternal

	/** the Dialogue jumps to the defined node, or the function returns with false, if the conversation is over */
	virtual bool EnterNode(int32 NodeIndex, TSet<const UDlgNode*> NodesEnteredWithThisStep) { return false; }

	virtual UDlgNode* GetActiveNode() { return nullptr; }
	virtual const UDlgNode* GetActiveNode() const { return nullptr; }

protected:
	/** Current Dialogue used in this context at runtime. */
	UPROPERTY()
	UDlgDialogue* Dialogue;

	/**
	 * All object is expected to implement the IDlgDialogueParticipant interface
	 * the key is the return value of IDlgDialogueParticipant::GetParticipantName()
	 */
	UPROPERTY()
	TMap<FName, UObject*> Participants;

	/** The index of the active node in the dialogues Nodes array */
	int32 ActiveNodeIndex = INDEX_NONE;

	/** Children of the active node with satisfied conditions - the options the player can choose from */
	TArray<const FDlgEdge*> AvailableChildren;

	/**
	 *  List of options which is possible, or would be with satisfied conditions
	 *  (e.g. in case of virtual parent it isn't necessary the node's child, that's why we have this array here
	 *  instead of simply returning something from active node
	 */
	TArray<FDlgEdgeData> AllChildren;

	/** Node indices visited in this specific Dialogue instance (isn't serialized) */
	TSet<int32> VisitedNodeIndices;
};
