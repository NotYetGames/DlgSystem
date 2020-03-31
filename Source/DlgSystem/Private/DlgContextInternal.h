// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgContext.h"
#include "DlgDialogue.h"
#include "DlgContextInternal.generated.h"

/**
 *  Context implementation, separated from the base class to provide a more clear interface
 *  the DialogueNodes uses the functionality of this class
 */
UCLASS(BlueprintType)
class DLGSYSTEM_API UDlgContextInternal : public UDlgContext
{
	GENERATED_BODY()
public:
	// Begin own methods

	/**
	 * Initializes the context, the first (start) node is selected and the first valid child node is entered.
	 * Called by the UDlgManager which creates the context
	 *
	 * @return true on success or false otherwise.
	 */
	bool Initialize(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants);

	/**
	 * Checks if the context could be initialized, used to check if there is any reachable node from the start node
	 *
	 * @return true if could be, false otherwise.
	 */
	bool CouldBeInitialized(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants);

	/**
	 * Initializes the context using the given node as entry point
	 *
	 * @return true on success or false otherwise.
	 */
	bool Initialize(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants, int32 StartIndex, const TSet<int32>& VisitedNodes, bool bFireEnterEvents);
};
