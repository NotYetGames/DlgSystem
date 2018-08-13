// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger/DlgGameplayDebuggerCategory.h"

#include "DebugRenderSceneProxy.h"

#include "DlgManager.h"

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
