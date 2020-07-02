// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgObject.h"
#include "DlgDialogue.h"
#include "Nodes/DlgNode.h"

#include "DlgContext.generated.h"

class USoundWave;
class USoundBase;
class UDialogueWave;
class UTexture2D;
class UDlgNodeData;
class UDlgNode;

// Used to store temporary state of edges
// This represents a const version of an Edge
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgEdgeData
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgEdgeData() {}
	FDlgEdgeData(bool bInSatisfied, const FDlgEdge& InEdge) : bSatisfied(bInSatisfied), Edge(InEdge) {};

	bool IsValid() const { return Edge.IsValid(); }
	bool IsSatisfied() const { return bSatisfied; }
	const FDlgEdge& GetEdge() const { return Edge; }

	static const FDlgEdgeData& GetInvalidEdge()
	{
		static FDlgEdgeData DlgEdge{false, FDlgEdge::GetInvalidEdge()};
		return DlgEdge;
	}

	// FDlgEdge& GetMutableEdge() { return *EdgePtr; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue|Edge")
	bool bSatisfied = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue|Edge")
	FDlgEdge Edge;
};


UENUM()
enum class EDlgValidateStatus : uint8
{
	Valid = 0,

	// Either the participant or dialogue is invalid
	ParticipantIsNull,
	DialogueIsNull,

	// Is an instance but does not implement the UDlgDialogueParticipant interface
	ParticipantDoesNotImplementInterface,

	// Is a blueprint class from the content browser and does not implement the UDlgDialogueParticipant interface
	ParticipantIsABlueprintClassAndDoesNotImplementInterface,

	// The Participant does not exist in the Dialogue
	// DialogueDoesNotContainParticipant
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
UCLASS(BlueprintType)
class DLGSYSTEM_API UDlgContext : public UDlgObject
{
	GENERATED_BODY()
public:

	//
	// UObject Interface
	//

	void PostInitProperties() override { Super::PostInitProperties(); }

	UDlgContext(const FObjectInitializer& ObjectInitializer);

	// Network support
	bool IsSupportedForNetworking() const override { return true; };
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//
	// Own methods
	//

	UFUNCTION()
    void OnRep_SerializedParticipants();
	void SerializeParticipants();

	/**
	 * Chooses the option with index OptionIndex of the active node index and it enters that node.
	 * Typically called based on user input.
	 * NOTICE: If the return value is false the dialogue is over and the context should be dropped
	 *
	 * @return true if the dialogue did not end, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control")
	bool ChooseChild(int32 OptionIndex);

	/**
	 *  Exactly as ChooseChild but expects an index from the AllOptions array
	 *  If the index is invalid or the selected edge is not satisfied the call fails
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control|All")
	bool ChooseChildBasedOnAllOptionIndex(int32 Index);

	/**
	 * Normally the children of the active node are checked only once, when the conversation enters the node.
	 * If an option can appear/disappear real time in the middle of the conversation this function should be called manually each frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control")
	void ReevaluateChildren();

	UFUNCTION(BlueprintPure, Category = "Dialogue|Control")
	bool HasDialogueEnded() const { return bDialogueEnded; }

	//
	// Use these functions if you don't care about unsatisfied player options:
	//

	// Gets the number of children with satisfied conditions (number of options)
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
	int32 GetOptionNum() const { return AvailableChildren.Num(); }

	// Gets the Text of the (satisfied) option with index OptionIndex
	// NOTE: This is just a helper method, you could have called GetOption
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
	const FText& GetOptionText(int32 OptionIndex) const;

	// Gets the SpeakerState of the (satisfied) edge with index OptionIndex
	// NOTE: This is just a helper method, you could have called GetOption
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
	FName GetOptionSpeakerState(int32 OptionIndex) const;

	// Gets the Enter Conditions of the (satisfied) edge with index OptionIndex
	// NOTE: This is just a helper method, you could have called GetOption
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
    const TArray<FDlgCondition>& GetOptionEnterConditions(int32 OptionIndex) const;

	// Gets the edge representing a player option from the satisfied options
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
	const FDlgEdge& GetOption(int32 OptionIndex) const;

	// Gets all satisfied edges
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
	const TArray<FDlgEdge>& GetOptionsArray() const { return AvailableChildren; }
	TArray<FDlgEdge>& GetMutableOptionsArray() { return AvailableChildren; }

	//
	//  Use these functions bellow if you don't care about unsatisfied player options:
	//  DO NOT missuse the indices above and bellow! The functions above expect < GetOptionNum(), bellow < GetAllOptionNum()
	//

	// Gets the number of children (both satisfied and unsatisfied ones are counted)
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	int32 GetAllOptionNum() const { return AllChildren.Num(); }

	// Gets the Text of an option from the all list, which includes the unsatisfied ones as well
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	const FText& GetOptionTextFromAll(int32 Index) const;

	// Is the option at Index satisfied? (Does it meet all the conditions)
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	bool IsOptionSatisfied(int32 Index) const;

	// Gets the SpeakerState of the edge with index OptionIndex
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	FName GetOptionSpeakerStateFromAll(int32 Index) const;

	// Gets the edge representing a player option from all options
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	const FDlgEdgeData& GetOptionFromAll(int32 Index) const;

	// Gets all edges (both satisfied and unsatisfied)
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	const TArray<FDlgEdgeData>& GetAllOptionsArray() const { return AllChildren; }
	TArray<FDlgEdgeData>& GetAllMutableOptionsArray() { return AllChildren; }

	/**
	*  Checks if the node connected directly to one of the active player choices was already visited or not
	*  Does not handle complicated logic - if the said node is a logical one it will still check that node, and not one
	*  of its children
	*
	* @param Index  Index of the edge/player option to test
	* @param bLocalHistory If true, only the history of this dialogue context is checked. If false, it is a global check
	* @param bIndexSkipsUnsatisfiedEdges  Decides if the index is in the [0, GetOptionNum()[ interval (if true), or in the [0, GetAllOptionNum()[ (if false)
	* @return true if the node was already IsOptionConnectedToVisitedNode
	*/
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
    bool IsOptionConnectedToVisitedNode(int32 Index, bool bLocalHistory = false, bool bIndexSkipsUnsatisfiedEdges = true) const;

	/**
	*  Checks if the node is connected directly to an end node or not
	*  Does not handle complicated logic - if the said node is a logical one it will still check that node, and not one
	*  of its children
	*
	* @param Index  Index of the edge/player option to test
	* @param bIndexSkipsUnsatisfiedEdges  Decides if the index is in the [0, GetOptionNum()[ interval (if true), or in the [0, GetAllOptionNum()[ (if false)
	* @return true if the node is an end node
	*/
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
    bool IsOptionConnectedToEndNode(int32 Index, bool bIndexSkipsUnsatisfiedEdges = true) const;


	//
	// Active Node
	//

	// Gets the Text of the active node index
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
    const FText& GetActiveNodeText() const;

	// Gets the SpeakerState of the active node index
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	FName GetActiveNodeSpeakerState() const;

	// Gets the Voice as a Sound Wave of the active node index
	// This will get cast to USoundWave from a USoundBase
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	USoundWave* GetActiveNodeVoiceSoundWave() const;

	// Same as GetActiveNodeVoiceSoundWave but this just returns the variable without casting it
	// to a USoundWave
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	USoundBase* GetActiveNodeVoiceSoundBase() const;

	// Gets the Voice as a Dialogue Wave of the active node index
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	UDialogueWave* GetActiveNodeVoiceDialogueWave() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	UObject* GetActiveNodeGenericData() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	UDlgNodeData* GetActiveNodeData() const;

	// Gets the Icon associated with the active node participant name (owner name).
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	UTexture2D* GetActiveNodeParticipantIcon() const;

	// Gets the Object associated with the active node participant name (owner name).
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	UObject* GetActiveNodeParticipant() const;

	// Gets the active node participant name (owner name).
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	FName GetActiveNodeParticipantName() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "GetParticipant")
	UObject* GetMutableParticipant(FName ParticipantName) const;
	const UObject* GetParticipant(FName ParticipantName) const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	const TMap<FName, UObject*>& GetParticipantsMap() const { return Participants; }

	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	int32 GetActiveNodeIndex() const { return ActiveNodeIndex; }

	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode", DisplayName = "GetActiveNode")
	UDlgNode* GetMutableActiveNode() const { return GetMutableNode(ActiveNodeIndex); }
	const UDlgNode* GetActiveNode() const { return GetNode(ActiveNodeIndex); }

	//
	// Data
	//

	// Returns the indices which were visited inside this single context. For global data check DlgMemory
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	const TSet<int32>& GetVisitedNodeIndices() const { return VisitedNodeIndices; }

	// Helper methods to get some Dialogue properties
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
    UDlgDialogue* GetDialogue() const { return Dialogue; }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	FName GetDialogueName() const { check(Dialogue); return Dialogue->GetDialogueFName(); }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	FGuid GetDialogueGUID() const { check(Dialogue); return Dialogue->GetDialogueGUID(); }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	FString GetDialoguePathName() const { check(Dialogue); return Dialogue->GetPathName(); }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	const TMap<FName, UObject*>& GetParticipants() const { return Participants; }


	// the Dialogue jumps to the defined node, or the function returns with false, if the conversation is over
	// the Dialogue jumps to the defined node, or the function returns with false if the conversation is over
	// Depending on the node the EnterNode() call can lead to other EnterNode() calls - having NodeIndex as active node after the call
	// is not granted
	// Conditions are not checked here - they are expected to be satisfied
	bool EnterNode(int32 NodeIndex, TSet<const UDlgNode*> NodesEnteredWithThisStep);

	// Gets the Node at the NodeIndex index
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "GetNode")
	UDlgNode* GetMutableNode(int32 NodeIndex) const;
	const UDlgNode* GetNode(int32 NodeIndex) const;

	// Was node with NodeIndex visited?
	UFUNCTION(BlueprintPure, Category = "Dialogue|Context")
	bool WasNodeVisitedInThisContext(int32 NodeIndex) const { return VisitedNodeIndices.Contains(NodeIndex); }

	// Checks the enter conditions of the node.
	// return false if they are not satisfied or if the index is invalid
	bool IsNodeEnterable(int32 NodeIndex, TSet<const UDlgNode*> AlreadyVisitedNodes) const;

	// Initializes/Starts the context, the first (start) node is selected and the first valid child node is entered.
	// Called by the UDlgManager which creates the context
	bool Start(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants) { return StartFromContext(TEXT(""), InDialogue, InParticipants); }
	bool StartFromContext(const FString& ContextString, UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants);

	// Initializes/Start the context using the given node as entry point
	// This is useful to resume a dialogue
	bool StartFromIndex(
		UDlgDialogue* InDialogue,
		const TMap<FName, UObject*>& InParticipants,
		int32 StartIndex,
		const TSet<int32>& VisitedNodes,
		bool bFireEnterEvents
	)
	{
		return StartFromContextFromIndex(
			TEXT(""),
			InDialogue,
			InParticipants,
			StartIndex,
			VisitedNodes,
			bFireEnterEvents
		);
	}
	bool StartFromContextFromIndex(
		const FString& ContextString,
	    UDlgDialogue* InDialogue,
	    const TMap<FName, UObject*>& InParticipants,
	    int32 StartIndex,
	    const TSet<int32>& VisitedNodes,
	    bool bFireEnterEvents
	);

	// Checks if the context could be started, used to check if there is any reachable node from the start node
	static bool CanBeStarted(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants);

	UFUNCTION(BlueprintPure, Category = "Dialogue|Context")
	FString GetContextString() const;

	// Checks if the Participant object is a valid participant for starting the Dialogue
	static EDlgValidateStatus IsValidParticipantForDialogue(const UDlgDialogue* Dialogue, const UObject* Participant);

	// Same as IsValidParticipantForDialogue but this just returns a bool and logs to the output log if something is wrong
	// If bLog = true then this act exactly as IsValidParticipantForDialogue
	static bool ValidateParticipantForDialogue(
		const FString& ContextString,
		const UDlgDialogue* Dialogue,
		const UObject* Participant,
		bool bLog = true
	);

	// Same as ValidateParticipantForDialogue but works on a Map of Participants
	static bool ValidateParticipantsMapForDialogue(
		const FString& ContextString,
		const UDlgDialogue* Dialogue,
		const TMap<FName, UObject*>& ParticipantsMap,
		bool bLog = true
	);

	// Just converts the array to a map, this does minimal checking just for the conversion to work
	// NOTE: this outputs to log if an error occurs
	static bool ConvertArrayOfParticipantsToMap(
		const FString& ContextString,
		const UDlgDialogue* Dialogue,
		const TArray<UObject*>& ParticipantsArray,
		TMap<FName, UObject*>& OutParticipantsMap,
		bool bLog = true
	);

protected:
	// bool StartInternal(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants, bool bLog, FString& OutErrorMessage);
	void LogErrorWithContext(const FString& ErrorMessage) const;
	FString GetErrorMessageWithContext(const FString& ErrorMessage) const;

	void SetParticipants(const TMap<FName, UObject*>& InParticipants)
	{
		Participants = InParticipants;
		SerializeParticipants();
	}

protected:
	// Current Dialogue used in this context at runtime.
	UPROPERTY(Replicated)
	UDlgDialogue* Dialogue = nullptr;

	//helper array to serialize to Participants map for clients as well
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_SerializedParticipants)
    TArray<UObject*> SerializedParticipants;

	// All object is expected to implement the IDlgDialogueParticipant interface
	// the key is the return value of IDlgDialogueParticipant::GetParticipantName()
	UPROPERTY()
	TMap<FName, UObject*> Participants;

	// The index of the active node in the dialogues Nodes array
	int32 ActiveNodeIndex = INDEX_NONE;

	// Children of the active node with satisfied conditions - the options the player can choose from
	TArray<FDlgEdge> AvailableChildren;

	/**
	 *  List of options which is possible, or would be with satisfied conditions
	 *  (e.g. in case of virtual parent it isn't necessary the node's child, that's why we have this array here
	 *  instead of simply returning something from active node
	 */
	TArray<FDlgEdgeData> AllChildren;

	// Node indices visited in this specific Dialogue instance (isn't serialized)
	TSet<int32> VisitedNodeIndices;

	// cache the result of the last ChooseChild call
	bool bDialogueEnded = false;
};
