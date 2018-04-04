// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgDialogue.h"
#include "DlgContext.generated.h"

struct FDlgEdge;
class USoundWave;
class UDialogueWave;

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
	UFUNCTION(BlueprintCallable, Category = DlgControl)
	virtual bool ChooseChild(int32 OptionIndex) { return false; }

	/**
	 * Normally the children of the active node are checked only once, when the conversation enters the node.
	 * If an option can appear/disappear real time in the middle of the conversation this function should be called manually each frame
	 */
	UFUNCTION(BlueprintCallable, Category = DlgControl)
	virtual void ReevaluateChildren() {}

	/** Gets the number of children with satisfied conditions (number of options) */
	UFUNCTION(BlueprintPure, Category = DlgData)
	int32 GetOptionNum() const { return AvailableChildren.Num(); }

	/** Gets the Text of the option with index OptionIndex  */
	UFUNCTION(BlueprintPure, Category = DlgData)
	const FText& GetOptionText(int32 OptionIndex) const;

	/** Gets the SpeakerState of the edge with index OptionIndex */
	UFUNCTION(BlueprintPure, Category = DlgData)
	FName GetOptionSpeakerState(int32 OptionIndex) const;

	/** Gets the Text of the active node index */
	UFUNCTION(BlueprintPure, Category = DlgData)
	const FText& GetActiveNodeText() const;

	/** Gets the SpeakerState of the active node index */
	UFUNCTION(BlueprintPure, Category = DlgData)
	FName GetActiveSpeakerState() const;

	/** Gets the Voice as a Sound Wave of the active node index */
	UFUNCTION(BlueprintPure, Category = DlgDataSound)
	USoundWave* GetActiveNodeVoiceSoundWave() const;

	/** Gets the Voice as a Dialogue Wave of the active node index */
	UFUNCTION(BlueprintPure, Category = DlgDataSound)
	UDialogueWave* GetActiveNodeVoiceDialogueWave() const;

	/** Gets the Icon associated with the active node participant name (owner name). */
	UFUNCTION(BlueprintPure, Category = DlgData)
	UTexture2D* GetActiveParticipantIcon() const;

	/** Gets the Object associated with the active node participant name (owner name). */
	UFUNCTION(BlueprintPure, Category = DlgData)
	UObject* GetActiveParticipant() const;

	UFUNCTION(BlueprintPure, Category = DlgData)
	UObject* GetParticipant(FName DlgParticipantName);

	const TMap<FName, UObject*>& GetParticipantMap() { return Participants; }

	UFUNCTION(BlueprintPure, Category = DlgData)
	int32 GetActiveNodeIndex() const { return ActiveNodeIndex; }
	
	/** Returns the indices which were visited inside this single context. For global data check DlgMemory */
	UFUNCTION(BlueprintPure, Category = DlgData)
	const TSet<int32>& GetVisitedNodeIndices() const { return VisitedNodeIndices; }

	/**
	 *  TODO: functions to get data from not satisfied edges - maybe sometimes some of those should be displayed as well (with different color?!)
	 *  Maybe edge should have a flag, if it is true it should be added to a special list if it is not satisfied?
	 *  E.g.: "Pay the 10$ fee" - grayed out because he/she/it does not have the money, but... maybe we want to show the player that it could be an option?!
	 *  we don't need this right now but if there is a request for it we should implement it
	 */

protected:
	// Methods implemented by UDlgContextInternal

	/** the Dialogue jumps to the defined node, or the function returns with false, if the conversation is over */
	virtual bool EnterNode(int32 NodeIndex, TSet<UDlgNode*> NodesEnteredWithThisStep) { return false; }

	virtual class UDlgNode* GetActiveNode() { return nullptr; }
	virtual const class UDlgNode* GetActiveNode() const { return nullptr; }

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

	/** The index of the active node in the dialogue's Nodes array */
	int32 ActiveNodeIndex = INDEX_NONE;

	/** Children of the active node with satisfied conditions - the options the player can choose from */
	TArray<const FDlgEdge*> AvailableChildren;

	/** Node indices visited in this specific Dialogue instance (isn't serialized) */
	TSet<int32> VisitedNodeIndices;
};
