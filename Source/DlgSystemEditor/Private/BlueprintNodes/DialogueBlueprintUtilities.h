// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "DlgDialogueParticipant.h"
#include "DlgManager.h"

class FDialogueBlueprintUtilities
{
public:
	/**
	 * Tries to get the dialogue name... it expects the owner of the node to implement IDlgDialogueParticipant interface
	 * @return		the participant name on success or NAME_None on failure.
	 */
	static FName GetParticipantNameFromNode(UK2Node* Node)
	{
		// NOTE we can't call Node->GetBlueprint() because this is called in strange places ;)
		if (UEdGraph* Graph = Cast<UEdGraph>(Node->GetOuter()))
		{
			if (UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph))
			{
				if (UDlgManager::DoesObjectImplementDialogueParticipantInterface(Blueprint))
				{
					return IDlgDialogueParticipant::Execute_GetParticipantName(Blueprint->GeneratedClass->GetDefaultObject());
				}
			}
		}

		return NAME_None;
	}
};
