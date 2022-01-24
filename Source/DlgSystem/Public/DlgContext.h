// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgObject.h"
#include "DlgDialogue.h"
#include "Nodes/DlgNode.h"
#include "DlgMemory.h"
#include "DlgParticipantName.h"

#include "DlgContext.generated.h"

class USoundWave;
class USoundBase;
class UDialogueWave;
class UTexture2D;
class UDlgNodeData;
class UDlgNode;
class UDlgNode_SpeechSequence;

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
 *  Call ChooseOption() if an option is selected
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

	UE_DEPRECATED(4.22, "ChooseChild has been deprecated in Favour of ChooseOption")
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control", meta = (DeprecatedFunction, DeprecationMessage = "ChooseChild has been deprecated in favour of ChooseOption"))
	bool ChooseChild(int32 OptionIndex) { return ChooseOption(OptionIndex); }

	/**
	* Chooses the option with index OptionIndex of the active node index and it enters that node.
	* Typically called based on user input.
	* NOTICE: If the return value is false the dialogue is over and the context should be dropped
	*
	* @return true if the dialogue did not end, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control")
	bool ChooseOption(int32 OptionIndex);

	/**
	 * Chooses the option with OptionIndex that is replicated
	 * NOTE: the ActiveNodeIndex must be a speech sequence node, otherwise the dialogue will end
	 *
	 * @return true if the dialogue did not end, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control")
	bool ChooseSpeechSequenceOptionFromReplicated(int32 OptionIndex);

	/**
	 *  Exactly as ChooseOption but expects an index from the AllOptions array
	 *  If the index is invalid the call fails
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control|All")
	bool ChooseOptionFromAll(int32 Index);

	UE_DEPRECATED(4.22, "ReevaluateChildren has been deprecated in Favour of ReevaluateOptions")
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control", meta = (DeprecatedFunction, DeprecationMessage = "ReevaluateChildren has been deprecated in Favour of ReevaluateOptions"))
	bool ReevaluateChildren() { return ReevaluateOptions(); }

	/**
	 * Normally the options of the active node are checked only once, when the conversation enters the node.
	 * If an option can appear/disappear real time in the middle of the conversation this function should be called manually each frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Control")
	bool ReevaluateOptions();

	UFUNCTION(BlueprintPure, Category = "Dialogue|Control")
	bool HasDialogueEnded() const { return bDialogueEnded; }

	//
	// Use these functions if you don't care about unsatisfied player options:
	//

	// Gets the number of options with satisfied conditions (number of options)
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
	int32 GetOptionsNum() const { return AvailableChildren.Num(); }

	// Is the OptionIndex valid index for the satisfied conditions?
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|Satisfied")
	bool IsValidOptionIndex(int32 OptionIndex) const { return AvailableChildren.IsValidIndex(OptionIndex);  }

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
	//  DO NOT missuse the indices above and bellow! The functions above expect < GetOptionsNum(), bellow < GetAllOptionsNum()
	//

	// Gets the number of options (both satisfied and unsatisfied ones are counted)
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	int32 GetAllOptionsNum() const { return AllChildren.Num(); }

	// Is the Index valid index for both satisfied and unsatisfied conditions
	UFUNCTION(BlueprintPure, Category = "Dialogue|Options|All")
	bool IsValidAllOptionIndex(int32 Index) const { return AllChildren.IsValidIndex(Index);  }

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
	*  of its options
	*
	* @param Index  Index of the edge/player option to test
	* @param bLocalHistory If true, only the history of this dialogue context is checked. If false, it is a global check
	* @param bIndexSkipsUnsatisfiedEdges  Decides if the index is in the [0, GetOptionsNum()[ interval (if true), or in the [0, GetAllOptionsNum()[ (if false)
	* @return true if the node was already IsOptionConnectedToVisitedNode
	*/
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	bool IsOptionConnectedToVisitedNode(int32 Index, bool bLocalHistory = false, bool bIndexSkipsUnsatisfiedEdges = true) const;

	/**
	*  Checks if the node is connected directly to an end node or not
	*  Does not handle complicated logic - if the said node is a logical one it will still check that node, and not one
	*  of its option
	*
	* @param Index  Index of the edge/player option to test
	* @param bIndexSkipsUnsatisfiedEdges  Decides if the index is in the [0, GetOptionsNum()[ interval (if true), or in the [0, GetAllOptionsNum()[ (if false)
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

	// Gets the active participant display name
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	FText GetActiveNodeParticipantDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "Get Participant")
	UObject* GetMutableParticipant(FName ParticipantName) const;
	const UObject* GetParticipant(FName ParticipantName) const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	const TMap<FName, UObject*>& GetParticipantsMap() const { return Participants; }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	UObject* GetParticipantFromName(const FDlgParticipantName& Participant);

	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	int32 GetActiveNodeIndex() const { return ActiveNodeIndex; }

	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode")
	FGuid GetActiveNodeGUID() const { return GetNodeGUIDForIndex(ActiveNodeIndex); }

	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode", DisplayName = "Get Active Node")
	UDlgNode* GetMutableActiveNode() const { return GetMutableNodeFromIndex(ActiveNodeIndex); }
	const UDlgNode* GetActiveNode() const { return GetNodeFromIndex(ActiveNodeIndex); }

	// Just a helper method for GetActiveNode that casts to UDlgNode_SpeechSequence
	UFUNCTION(BlueprintPure, Category = "Dialogue|ActiveNode", DisplayName = "Get Active Node As Speech Sequence")
	UDlgNode_SpeechSequence* GetMutableActiveNodeAsSpeechSequence() const;
	const UDlgNode_SpeechSequence* GetActiveNodeAsSpeechSequence() const;

	//
	// Data
	//

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	bool IsValidNodeIndex(int32 NodeIndex) const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	bool IsValidNodeGUID(const FGuid& NodeGUID) const;

	// Gets the GUID for the Node at NodeIndex
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "Get Node GUID For Index")
	FGuid GetNodeGUIDForIndex(int32 NodeIndex) const;

	// Gets the corresponding Node Index for the supplied NodeGUID
	// Returns -1 (INDEX_NONE) if the Node GUID does not exist.
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "Get Node Index For GUID")
	int32 GetNodeIndexForGUID(const FGuid& NodeGUID) const;

	// Returns the indices which were visited inside this single context. For global data check DlgMemory
	// NOTE: You should use GetVisitedNodeGUIDs
	UFUNCTION(BlueprintPure, Category = "Dialogue|Context|History")
	const TSet<int32>& GetVisitedNodeIndices() const { return History.VisitedNodeIndices; }

	// Returns the GUIDs which were visited inside this single context. For global data check DlgMemory
	UFUNCTION(BlueprintPure, Category = "Dialogue|Context|History")
	const TSet<FGuid>& GetVisitedNodeGUIDs() const { return History.VisitedNodeGUIDs; }

	// Helper methods to get some Dialogue properties
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	UDlgDialogue* GetDialogue() const { return Dialogue; }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	FName GetDialogueName() const { check(Dialogue); return Dialogue->GetDialogueFName(); }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	FGuid GetDialogueGUID() const { check(Dialogue); return Dialogue->GetGUID(); }

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

	// Adds the node as visited in the current dialogue memory
	virtual void SetNodeVisited(int32 NodeIndex, const FGuid& NodeGUID);

	UFUNCTION(BlueprintPure, Category = "Dialogue|Context|History")
	virtual bool IsNodeVisited(int32 NodeIndex, const FGuid& NodeGUID, bool bLocalHistory) const;

	virtual FDlgNodeSavedData& GetNodeSavedData(const FGuid& NodeGUID);

	// Gets the Node at the NodeIndex index
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "Get Node From Index")
	UDlgNode* GetMutableNodeFromIndex(int32 NodeIndex) const;
	const UDlgNode* GetNodeFromIndex(int32 NodeIndex) const;

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "Get Node From GUID")
	UDlgNode* GetMutableNodeFromGUID(const FGuid& NodeGUID) const;
	const UDlgNode* GetNodeFromGUID(const FGuid& NodeGUID) const;

	// Was the node Index visited in the lifetime of this context?
	// NOTE: you should  most likely use WasNodeGUIDVisitedInThisContext
	UFUNCTION(BlueprintPure, Category = "Dialogue|Context|History", DisplayName = "Was Node Index Visited In This Context")
	bool WasNodeIndexVisitedInThisContext(int32 NodeIndex) const
	{
		return History.VisitedNodeIndices.Contains(NodeIndex);
	}

	// Was the node GUID visited in the lifetime of this context?
	UFUNCTION(BlueprintPure, Category = "Dialogue|Context|History", DisplayName = "Was Node GUID Visited In This Context")
	bool WasNodeGUIDVisitedInThisContext(const FGuid& NodeGUID) const
	{
		return History.VisitedNodeGUIDs.Contains(NodeGUID);
	}

	// Gets the History of this context
	const FDlgHistory& GetHistoryOfThisContext() const { return History; }

	// Checks the enter conditions of the node.
	// return false if they are not satisfied or if the index is invalid
	bool IsNodeEnterable(int32 NodeIndex, TSet<const UDlgNode*> AlreadyVisitedNodes) const;

	// Initializes/Starts the context, the first (start) node is selected and the first valid child node is entered.
	// Called by the UDlgManager which creates the context
	bool Start(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants) { return StartWithContext(TEXT(""), InDialogue, InParticipants); }
	bool StartWithContext(const FString& ContextString, UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants);

	//
	// Initializes/Start the context using the given node as entry point
	// This is useful to resume a dialogue
	//

	// Variant that works with only the NodeIndex
	// NOTE: This is not safe, please use StartFromNodeGUID
	bool StartFromNodeIndex(
		UDlgDialogue* InDialogue,
		const TMap<FName, UObject*>& InParticipants,
		int32 StartNodeIndex,
		const FDlgHistory& StartHistory,
		bool bFireEnterEvents
	)
	{
		return StartWithContextFromNodeIndex(
			TEXT(""),
			InDialogue,
			InParticipants,
			StartNodeIndex,
			StartHistory,
			bFireEnterEvents
		);
	}
	bool StartWithContextFromNodeIndex(
		const FString& ContextString,
		UDlgDialogue* InDialogue,
		const TMap<FName, UObject*>& InParticipants,
		int32 StartNodeIndex,
		const FDlgHistory& StartHistory,
		bool bFireEnterEvents
	)
	{
		const FString ContextMessage = ContextString.IsEmpty()
			? TEXT("StartFromNodeIndex")
			: FString::Printf(TEXT("%s - StartFromNodeIndex"), *ContextString);

		return StartWithContextFromNode(
			ContextMessage,
			InDialogue,
			InParticipants,
			StartNodeIndex,
			FGuid{},
			StartHistory,
			bFireEnterEvents
		);
	}

	// Variant that works with only the NodeGUID
	bool StartFromNodeGUID(
		UDlgDialogue* InDialogue,
		const TMap<FName, UObject*>& InParticipants,
		const FGuid& StartNodeGUID,
		const FDlgHistory& StartHistory,
		bool bFireEnterEvents
	)
	{
		return StartWithContextFromNodeGUID(
			TEXT(""),
			InDialogue,
			InParticipants,
			StartNodeGUID,
			StartHistory,
			bFireEnterEvents
		);
	}
	bool StartWithContextFromNodeGUID(
		const FString& ContextString,
		UDlgDialogue* InDialogue,
		const TMap<FName, UObject*>& InParticipants,
		const FGuid& StartNodeGUID,
		const FDlgHistory& StartHistory,
		bool bFireEnterEvents
	)
	{
		const FString ContextMessage = ContextString.IsEmpty()
			? TEXT("StartFromNodeGUID")
			: FString::Printf(TEXT("%s - StartFromNodeGUID"), *ContextString);

		return StartWithContextFromNode(
			ContextMessage,
			InDialogue,
			InParticipants,
			INDEX_NONE,
			StartNodeGUID,
			StartHistory,
			bFireEnterEvents
		);
	}

	// Generic variant that accepts both NodeIndex and NodeGUID
	// If NodeGUID is valid this will be used to get the correct Node
	// Otherwise fallback to the NodeIndex
	bool StartFromNode(
		UDlgDialogue* InDialogue,
		const TMap<FName, UObject*>& InParticipants,
		int32 StartNodeIndex,
		const FGuid& StartNodeGUID,
		const FDlgHistory& StartHistory,
		bool bFireEnterEvents
	)
	{
		return StartWithContextFromNode(
			TEXT(""),
			InDialogue,
			InParticipants,
			StartNodeIndex,
			StartNodeGUID,
			StartHistory,
			bFireEnterEvents
		);
	}
	bool StartWithContextFromNode(
		const FString& ContextString,
		UDlgDialogue* InDialogue,
		const TMap<FName, UObject*>& InParticipants,
		int32 StartNodeIndex,
		const FGuid& StartNodeGUID,
		const FDlgHistory& StartHistory,
		bool bFireEnterEvents
	);

	// Create a copy of the current Context
	UDlgContext* CreateCopy() const;

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

	// Helper array to serialize to Participants map for clients as well
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_SerializedParticipants)
	TArray<UObject*> SerializedParticipants;

	// All object is expected to implement the IDlgDialogueParticipant interface
	// the key is the return value of IDlgDialogueParticipant::GetParticipantName()
	UPROPERTY()
	TMap<FName, UObject*> Participants;

	// The index of the active node in the dialogues Nodes array
	int32 ActiveNodeIndex = INDEX_NONE;

	// Options of the active node with satisfied conditions - the options the player can choose from
	TArray<FDlgEdge> AvailableChildren;

	/**
	 *  List of options which is possible, or would be with satisfied conditions
	 *  (e.g. in case of virtual parent it isn't necessary the node's child, that's why we have this array here
	 *  instead of simply returning something from active node
	 */
	TArray<FDlgEdgeData> AllChildren;

	// Node indices visited in this specific Dialogue instance (isn't serialized)
	// History for this Context only
	FDlgHistory History;

	// cache the result of the last ChooseOption call
	bool bDialogueEnded = false;
};
