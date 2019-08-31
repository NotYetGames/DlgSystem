// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "UObject/Object.h"

#include "DlgNodeData.generated.h"

/**
 *  Abstract base class for tasks
 *  A task can be either a single action or an action executed for a specified duration
 *  Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, Abstract)
class DLGSYSTEM_API UDlgNodeData : public UObject
{
	GENERATED_BODY()
};
