// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"

class AActor;
class APlayerController;
class FGameplayDebuggerCanvasContext;

// The data we're going to print inside the viewport
struct DLGSYSTEM_API FDlgDataToPrint
{
	int32 NumLoadedDialogues = 0;
};

class DLGSYSTEM_API FDlgGameplayDebuggerCategory : public FGameplayDebuggerCategory
{
private:
	typedef FDlgGameplayDebuggerCategory Self;

public:
	FDlgGameplayDebuggerCategory();

	/** Creates an instance of this category - will be used on module startup to include our category in the Editor */
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance() { return MakeShared<Self>(); }

	// Begin FGameplayDebuggerCategory Interface

	/** Collects the data we would like to print */
	void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	/** Displays the data we collected in the CollectData function */
	void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

protected:
	// The data that we're going to print
	FDlgDataToPrint Data;
};

#endif // WITH_GAMEPLAY_DEBUGGER
