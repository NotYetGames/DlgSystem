// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Factories/Factory.h"

#include "DlgAssetFactory.generated.h"

/**
 * Factory for dialogues. Editor does the magic here, without this class,
 * you won't have the right click "Dialog System" -> "Dialogue"
 */
UCLASS()
class DLGSYSTEMEDITOR_API UDlgAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDlgAssetFactory(const FObjectInitializer& ObjectInitializer);

	//
	// UFactory interface
	//
	UObject* FactoryCreateNew(
		UClass* Class,
		UObject* InParent,
		FName Name,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn
	) override;
};
