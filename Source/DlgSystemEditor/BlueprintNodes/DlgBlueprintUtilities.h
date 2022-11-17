// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "DlgSystem/DlgDialogueParticipant.h"
#include "DlgSystem/DlgManager.h"

class FDlgBlueprintUtilities
{
public:
	// Gets the blueprint for the provided Node
	static UBlueprint* GetBlueprintForGraphNode(const UK2Node* Node)
	{
		if (!IsValid(Node))
		{
			return nullptr;
		}

		// NOTE we can't call Node->GetBlueprint() because this is called in strange places ;)
		if (const UEdGraph* Graph = Cast<UEdGraph>(Node->GetOuter()))
		{
			return FBlueprintEditorUtils::FindBlueprintForGraph(Graph);
		}

		return nullptr;
	}

	// Checks if the Blueprint for the Node is loaded or not.
	static bool IsBlueprintLoadedForGraphNode(const UK2Node* Node)
	{
		return IsBlueprintLoaded(GetBlueprintForGraphNode(Node));
	}
	static bool IsBlueprintLoaded(const UBlueprint* Blueprint)
	{
		return Blueprint ? !Blueprint->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad) : false;
	}

	/**
	 * Tries to get the dialogue name... it expects the owner of the node to implement IDlgDialogueParticipant interface
	 * @return		the participant name on success or NAME_None on failure.
	 */
	static FName GetParticipantNameFromNode(const UK2Node* Node, bool bBlueprintMustBeLoaded)
	{
		if (const UBlueprint* Blueprint = GetBlueprintForGraphNode(Node))
		{
			if (bBlueprintMustBeLoaded && !IsBlueprintLoaded(Blueprint))
			{
				return NAME_None;
			}

			if (UDlgManager::DoesObjectImplementDialogueParticipantInterface(Blueprint))
			{
				return IDlgDialogueParticipant::Execute_GetParticipantName(Blueprint->GeneratedClass->GetDefaultObject());
			}
		}

		return NAME_None;
	}
};
