// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Factories/Factory.h"

#include "DlgEventCustomFactory.generated.h"

class UDlgEventCustom;

UCLASS()
class DLGSYSTEMEDITOR_API UDlgEventCustomFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDlgEventCustomFactory(const FObjectInitializer& ObjectInitializer);

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
	TSubclassOf<UDlgEventCustom> ParentClass;
};
