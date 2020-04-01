// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgConditionCustom.h"

#include "DlgManager.h"
#include "UObject/Object.h"

UWorld* UDlgConditionCustom::GetWorld() const
{
	if (HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
	{
		return nullptr;
	}

	// Get from outer
	if (UObject* Outer = GetOuter())
	{
		if (UWorld* World = Outer->GetWorld())
		{
			return World;
		}
	}

	// Last resort
	return UDlgManager::GetDialogueWorld();
}
