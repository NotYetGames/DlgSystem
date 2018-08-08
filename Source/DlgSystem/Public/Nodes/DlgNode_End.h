// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "Nodes/DlgNode.h"

#include "DlgNode_End.generated.h"


/**
 * Node ending the Dialogue.
 * Does not have text, if it is entered the Dialogue is over.
 * Events and enter conditions are taken into account.
 */
UCLASS(BlueprintType)
class DLGSYSTEM_API UDlgNode_End : public UDlgNode
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.

	/** @return a one line description of an object. */
	FString GetDesc() override;

	// Begin UDlgNode Interface.
	bool ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyEvaluated) override { return false; }
	bool OptionSelected(int32 OptionIndex, UDlgContextInternal* DlgContext) override { return false; }

#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("End"); }
#endif
};
