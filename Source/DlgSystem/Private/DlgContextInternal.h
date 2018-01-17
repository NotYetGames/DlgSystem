// Copyright 2017-2018 Csaba Molnar, Daniel Butum
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

	/**
	 * Initializes the context, the first (start) node is selected and the first valid child node is entered.
	 * Called by the UDlgManager which creates the context
	 *
	 * @return true on success or false otherwise.
	 */
	bool Initialize(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants);

	/**
	 *  Implementation of EnterNode()
	 *  the Dialogue jumps to the defined node, or the function returns with false if the conversation is over
	 *  Depending on the node the EnterNode() call can lead to other EnterNode() calls - having NodeIndex as active node after the call
	 *  is not granted
	 *  Conditions are not checked here - they are expected to be satisfied
	 */
	virtual bool EnterNode(int32 NodeIndex, TSet<UDlgNode*> NodesEnteredWithThisStep) override;

	/**
	 *  Called when an option is selected
	 *  Dialogue progression depends on the actual node
	 */
	virtual bool ChooseChild(int32 OptionIndex) override;


	/** Checks the enter conditions of the node, return false if they are not satisfied or if the index is invalid */
	bool IsNodeEnterable(int32 NodeIndex, TSet<UDlgNode*> AlreadyVisitedNodes);


	/** Called from game if a condition change could lead to different satisfied options for the active node */
	virtual void ReevaluateChildren() override;

	// Helper methods to get some Dialogue properties
	FName GetDialogueName() const { check(Dialogue); return Dialogue->GetDlgFName(); }
	FGuid GetDialogueGuid() const { check(Dialogue); return Dialogue->GetDlgGuid();  }

	TArray<const FDlgEdge*>& GetOptionArray() { return AvailableChildren; }
	const TMap<FName, UObject*>& GetParticipants() const { return Participants; }

	virtual UDlgNode* GetActiveNode() override { return GetNode(ActiveNodeIndex); }
	virtual const UDlgNode* GetActiveNode() const override { return GetNode(ActiveNodeIndex); }

	UDlgNode* GetNode(int32 NodeIndex);
	const UDlgNode* GetNode(int32 NodeIndex) const;

	bool WasNodeVisitedInThisContext(const int32 NodeIndex) const { return VisitedNodeIndices.Contains(NodeIndex); }

protected:

	virtual UDlgContextInternal* GetSelfAsInternal() override { return this; }
	virtual const UDlgContextInternal* GetSelfAsInternal() const override { return this; }

	/** Node indices visited in this specific Dialogue instance (isn't serialized) */
	TSet<int32> VisitedNodeIndices;
};
