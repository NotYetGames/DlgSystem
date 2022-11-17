// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "DlgSystem/Nodes/DlgNode.h"

#include "DlgNode_Start.generated.h"


/**
 * Possible entry point of the Dialogue.
 * Does not have text, the first satisfied child is picked if there is any.
 * Start nodes are evaluated from left to right.
 */
UCLASS(BlueprintType, ClassGroup = "Dialogue")
class DLGSYSTEM_API UDlgNode_Start : public UDlgNode
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.

	/** @return a one line description of an object. */
	FString GetDesc() override;

#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Start"); }
#endif
};
