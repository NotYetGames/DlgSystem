// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Factories/Factory.h"

#include "DlgNodeDataFactory.generated.h"

class UDlgNodeData;

UCLASS()
class DLGSYSTEMEDITOR_API UDlgNodeDataFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDlgNodeDataFactory(const FObjectInitializer& ObjectInitializer);

	//
	// UFactory interface
	//

	bool ConfigureProperties() override;
	UObject* FactoryCreateNew(
		UClass* Class,
		UObject* InParent,
		FName Name,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn
	) override;

private:
	// Holds the template of the class we are building
	UPROPERTY()
	TSubclassOf<UDlgNodeData> ParentClass;
};
