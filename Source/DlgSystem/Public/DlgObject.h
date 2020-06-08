// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "UObject/Object.h"

#include "DlgObject.generated.h"

// Our Dialogue base object
UCLASS(Abstract, ClassGroup = "Dialogue", HideCategories = ("DoNotShow"), AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgObject : public UObject
{
	GENERATED_BODY()
public:
	// UObject interface
	void PostInitProperties() override;
	UWorld* GetWorld() const override;
};
