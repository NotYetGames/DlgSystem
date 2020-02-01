// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"

#include "DlgDialogue.h"
#include "DlgDialogueParticipant.h"
#include "DlgMemory.h"

#include "DlgManager.generated.h"

class AActor;
class UDlgContext;
class UDlgDialogue;


/**
 *  Class providing a collection of static functions to start a conversation and work with Dialogues.
 */
UCLASS()
class DLGSYSTEM_API UDlgManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Starts a Dialogue with the provided Dialogue
	 * The function checks all the objects in the world to gather the participants
	 * This method can fail in the following situations:
	 *  - The Dialogue has a Participant which does not exist in the World
	 *	- Multiple Objects are using the same Participant Name in the World
	 *
	 * @returns The dialogue context object or nullptr if something went wrong
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch", meta = (WorldContext = "WorldContextObject"))
	static UDlgContext* StartDialogueWithDefaultParticipants(UObject* WorldContextObject, UDlgDialogue* Dialogue);


	/**
	 * Starts a Dialogue with the provided Dialogue and Participants array
	 * This method can fail in the following situations:
	 *  - The Participants number does not match the number of participants from the Dialogue.
	 *  - Any UObject in the Participant array does not implement the Participant Interface
	 *  - Participant->GetParticipantName() does not exist in the Dialogue
	 *
	 * @returns The dialogue context object or nullptr if something wrong happened
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch")
	static UDlgContext* StartDialogue(UDlgDialogue* Dialogue, UPARAM(ref)const TArray<UObject*>& Participants);

	/**
	 * Checks if there is any child of the start node which can be enterred based on the conditions
	 *
	 * @returns true if there is an enterable node from the start node
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch")
	static bool CouldStartDialogue(UDlgDialogue* Dialogue, UPARAM(ref)const TArray<UObject*>& Participants);

	/**
	 * Starts a Dialogue with the provided Dialogue and Participants array, at the given entry point
	 * This method can fail in the following situations:
	 *  - The Participants number does not match the number of participants from the Dialogue.
	 *  - Any UObject in the Participant array does not implement the Participant Interface
	 *  - Participant->GetParticipantName() does not exist in the Dialogue
	 *  - The given node index is invalid
	 *  - The starter node does not have any valid child
	 *
	 * @param Dialogue				- The dialogue asset to start
	 * @param Participants			- Array of participants, has to match with the expected input for the Dialogue
	 * @param StartIndex			- Index of the node the dialogue is resumed at
	 * @param AlreadyVisitedNodes	- Set of nodes already visited in the context the last time this Dialogue was going on.
	 *								  Can be aquired via GetVisitedNodeIndices() on the context
	 * @param bFireEnterEvents		- decides if the enter events should be fired on the resumed node or not
	 * @returns The dialogue context object or nullptr if something wrong happened
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch")
	static UDlgContext* ResumeDialogue(UDlgDialogue* Dialogue, UPARAM(ref)const TArray<UObject*>& Participants,
									   int32 StartIndex, const TSet<int32>& AlreadyVisitedNodes, bool bFireEnterEvents);


	/**
	 * Helper methods, same as StartDialogue but with fixed amount of participant(s)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch")
	static UDlgContext* StartMonologue(UDlgDialogue* Dialogue, UObject* Participant);

	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch")
	static UDlgContext* StartDialogue2(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1);

	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch")
	static UDlgContext* StartDialogue3(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2);

	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch")
	static UDlgContext* StartDialogue4(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2, UObject* Participant3);

	/**
	 * Loads all dialogues from the filesystem into memory
	 * @return number of loaded dialogues
	 */
	static int32 LoadAllDialoguesIntoMemory();

	/** Gets all loaded dialogues from memory. LoadAllDialoguesIntoMemory must be called before this */
	static TArray<UDlgDialogue*> GetAllDialoguesFromMemory();

	/** Gets all the objects from the provided World that implement the Dialogue Participant Interface. Iterates through all objects, DO NOT CALL EACH FRAME */
	static TArray<TWeakObjectPtr<AActor>> GetAllActorsImplementingDialogueParticipantInterface(UWorld* World);

	/** Gets all objects from the World that implement the Dialogue Participant Interface */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Helper", meta = (WorldContext = "WorldContextObject"))
	static TArray<UObject*> GetAllObjectsWithDialogueParticipantInterface(UObject* WorldContextObject);

	/** Gets all the dialogues that have a duplicate GUID, should not happen, like ever. */
	static TArray<UDlgDialogue*> GetDialoguesWithDuplicateGuid();

	/** Helper methods that gets all the dialouges in a map by guid. */
	static TMap<FGuid, UDlgDialogue*> GetAllDialoguesGuidMap();

	/** Gets all the loaded dialogues from memory that have the ParticipantName included inside them. */
	static TArray<UDlgDialogue*> GetAllDialoguesForParticipantName(const FName& ParticipantName);

	/** Sets the FDlgMemory Dialogue history. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Memory")
	static void SetDialogueHistory(const TMap<FGuid, FDlgHistory>& DlgHistory);

	/** Empties the FDlgMemory Dialogue history. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Memory")
	static void ClearDialogueHistory();

	/** Gets the Dialogue History from the FDlgMemory. */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Memory")
	static const TMap<FGuid, FDlgHistory>& GetDialogueHistory();

	/** Does the Object implement the Dialogue Participant Interface? */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Helper")
	static bool DoesObjectImplementDialogueParticipantInterface(const UObject* Object);

	/** Gets all the unique participant names sorted alphabetically from all the Dialogues loaded into memory. */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesParticipantNames(TArray<FName>& OutArray);

	/** Gets all the used speaker states sorted alphabetically from all the Dialogues loaded into memory. */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesSpeakerStates(TArray<FName>& OutArray);

	/** Gets all the unique int variable names sorted alphabetically for the specified ParticipantName from the loaded Dialogues */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesIntNames(const FName& ParticipantName, TArray<FName>& OutArray);

	/** Gets all the unique float variable names sorted alphabetically for the specified ParticipantName from the loaded Dialogues */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesFloatNames(const FName& ParticipantName, TArray<FName>& OutArray);

	/** Gets all the unique bool variable names sorted alphabetically for the specified ParticipantName from the loaded Dialogues */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesBoolNames(const FName& ParticipantName, TArray<FName>& OutArray);

	/** Gets all the unique name variable names sorted alphabetically for the specified ParticipantName from the loaded Dialogues */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesNameNames(const FName& ParticipantName, TArray<FName>& OutArray);

	/** Gets all the unique condition names sorted alphabetically for the specified ParticipantName from the loaded Dialogues */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesConditionNames(const FName& ParticipantName, TArray<FName>& OutArray);

	/** Gets all the unique event names sorted alphabetically for the specified ParticipantName from the loaded Dialogues */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Data")
	static void GetAllDialoguesEventNames(const FName& ParticipantName, TArray<FName>& OutArray);

	/**
	 * Registers all the DlgSystem Module console commands.
	 * @param InReferenceActor - The reference actor for the World. Without this the runtime module won't know how to get the UWorld.
	 * @return true on success, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Module")
	static bool RegisterDialogueModuleConsoleCommands(AActor* InReferenceActor);

	/**
	 * Unregister all the DlgSystem Module console commands.
	 * @return true on success, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Module")
	static bool UnRegisterDialogueModuleConsoleCommands();

private:
	static bool ValidateParticipant(const FString& ContextMessageFailure, const UDlgDialogue* ContextDialogue, UObject* Participant);
	static bool ConstructParticipantMap(const UDlgDialogue* Dialogue, const TArray<UObject*>& Participants, TMap<FName, UObject*>& OutMap);

	static void GatherParticipantsRecursive(UObject* Object, TArray<UObject*>& Array, TSet<UObject*>& AlreadyVisited);
};
