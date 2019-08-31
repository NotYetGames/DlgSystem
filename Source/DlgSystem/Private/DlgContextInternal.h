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
	typedef UDlgContextInternal Self;
public:
	// Begin UDlgContext Interface

	/**
	 *  Implementation of EnterNode()
	 *  the Dialogue jumps to the defined node, or the function returns with false if the conversation is over
	 *  Depending on the node the EnterNode() call can lead to other EnterNode() calls - having NodeIndex as active node after the call
	 *  is not granted
	 *  Conditions are not checked here - they are expected to be satisfied
	 */
	bool EnterNode(int32 NodeIndex, TSet<const UDlgNode*> NodesEnteredWithThisStep) override;

	/**
	 *  Called when an option is selected
	 *  Dialogue progression depends on the actual node
	 */
	bool ChooseChild(int32 OptionIndex) override;


	/** Called from game if a condition change could lead to different satisfied options for the active node */
	void ReevaluateChildren() override;

	// gets the active node
	UDlgNode* GetActiveNode() override { return GetNode(ActiveNodeIndex); }
	const UDlgNode* GetActiveNode() const override { return GetNode(ActiveNodeIndex); }

	// End UDlgContext Interface

	// Begin own methods

	/**
	 * Initializes the context, the first (start) node is selected and the first valid child node is entered.
	 * Called by the UDlgManager which creates the context
	 *
	 * @return true on success or false otherwise.
	 */
	bool Initialize(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants);

	/**
	 * Initializes the context using the given node as entry point
	 *
	 * @return true on success or false otherwise.
	 */
	bool Initialize(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants, int32 StartIndex, const TSet<int32>& VisitedNodes, bool bFireEnterEvents);


	/** Checks the enter conditions of the node, return false if they are not satisfied or if the index is invalid */
	bool IsNodeEnterable(int32 NodeIndex, TSet<const UDlgNode*> AlreadyVisitedNodes) const;

	TArray<const FDlgEdge*>& GetOptionArray() { return AvailableChildren; }
	TArray<FDlgEdgeData>& GetAllOptionsArray() { return AllChildren; }
	const TMap<FName, UObject*>& GetParticipants() const { return Participants; }

	// Gets the Node at the NodeIndex index
	UDlgNode* GetNode(int32 NodeIndex)
	{
		return const_cast<UDlgNode*>(const_cast<const Self*>(this)->GetNode(NodeIndex));
	}
	const UDlgNode* GetNode(int32 NodeIndex) const;

	// Was node with NodeIndex visited?
	bool WasNodeVisitedInThisContext(int32 NodeIndex) const { return VisitedNodeIndices.Contains(NodeIndex); }
};
