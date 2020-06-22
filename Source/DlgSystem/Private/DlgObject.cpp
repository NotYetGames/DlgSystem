// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgObject.h"

#include "DlgManager.h"
#include "UObject/Object.h"

void UDlgObject::PostInitProperties()
{
	// We must always set the outer to be something that exists at runtime
#if WITH_EDITOR
	if (UEdGraphNode* GraphNode =  Cast<UEdGraphNode>(GetOuter()))
	{
		UDlgDialogue::GetDialogueEditorAccess()->SetNewOuterForObjectFromGraphNode(this, GraphNode);
	}
#endif

	Super::PostInitProperties();
}

UWorld* UDlgObject::GetWorld() const
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
