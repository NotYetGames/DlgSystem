// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#if WITH_GAMEPLAY_DEBUGGER
#include "DlgGameplayDebuggerCategory.h"

#include "DlgSystem/DlgManager.h"

FDlgGameplayDebuggerCategory::FDlgGameplayDebuggerCategory()
{
	bShowOnlyWithDebugActor = false;
}

void FDlgGameplayDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	Data.NumLoadedDialogues = UDlgManager::GetAllDialoguesFromMemory().Num();

	if (DebugActor)
	{

	}
}

void FDlgGameplayDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	CanvasContext.Printf(TEXT("{green}Number loaded Dialogues: %s"), *FString::FromInt(Data.NumLoadedDialogues));
}

#endif // WITH_GAMEPLAY_DEBUGGER
