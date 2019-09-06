// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "UObject/Object.h"

#include "DlgNodeData.generated.h"

/**
 *  Abstract base class for node data
 *  Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
 *  Extend this class to define additional data you want to store on your nodes
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, Abstract)
class DLGSYSTEM_API UDlgNodeData : public UObject
{
	GENERATED_BODY()
};
