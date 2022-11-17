// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

#include "DlgSystem/Nodes/DlgNode.h"

#include "DlgNode_Proxy.generated.h"



/**
 * Node without text. Execution auto-steps through it to the referenced node.
 * Can be used to reduce dialogue graph spaghetti.
 */
UCLASS(BlueprintType, ClassGroup = "Dialogue")
class DLGSYSTEM_API UDlgNode_Proxy : public UDlgNode
{
	GENERATED_BODY()

public:

	// @return a one line description of an object.
	FString GetDesc() override
	{
		return TEXT("Jumps execution to the specified target.");
	}

	//
	// Begin UDlgNode Interface.
	//

	bool HandleNodeEnter(UDlgContext& Context, TSet<const UDlgNode*> NodesEnteredWithThisStep) override;
	virtual bool CheckNodeEnterConditions(const UDlgContext& Context, TSet<const UDlgNode*> AlreadyVisitedNodes) const override;

#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Proxy"); }
#endif

	//
	// Begin own functions
	//

	/** Called when dialogue indicies are rebuilt, updates the index so it points to the same node after the change */
	void RemapOldIndicesWithNew(const TMap<int32, int32>& OldToNewIndexMap);

	// return with the index of the target in the UDlgDialogue::Nodes array
	int32 GetTargetNodeIndex() const { return NodeIndex; }


	// Helper functions to get the names of some properties. Used by the DlgSystemEditor module.
	static FName GetMemberNameNodeIndex() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Proxy, NodeIndex); }

protected:

	// Index of the node the Proxy represents (in UDlgDialogue::Nodes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 NodeIndex = 0;
};
