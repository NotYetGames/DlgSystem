// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DialogueK2Node_Select.h"
#include "DialogueK2Node_SwitchDialogueCallback.h"
#include "DialogueK2Node_SwitchDialogueCallbackVariants.h"

#include "DialogueK2Node_Redirects.generated.h"

// Make the node not appear in the menus
// So that the user creates only new nodes, instead of old
#define MAKE_NODE_FITERED_OUT() bool IsActionFilteredOut(FBlueprintActionFilter const& Filter) override { return true; }

// Selects
UCLASS()
class UDlgK2Node_Select : public UDialogueK2Node_Select
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

UCLASS()
class UDlgK2Node_SelectFloat : public UDialogueK2Node_SelectFloat
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

// Switches
UCLASS()
class UDlgK2Node_SwitchDialogueCallback : public UDialogueK2Node_SwitchDialogueCallback
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

UCLASS()
class UDlgK2Node_SwitchDialogueCallbackEvent : public UDialogueK2Node_SwitchDialogueCallbackEvent
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

UCLASS()
class UDlgK2Node_SwitchDialogueCallbackCondition : public UDialogueK2Node_SwitchDialogueCallbackCondition
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

UCLASS()
class UDlgK2Node_SwitchDialogueCallbackFloatValue : public UDialogueK2Node_SwitchDialogueCallbackFloatValue
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

UCLASS()
class UDlgK2Node_SwitchDialogueCallbackIntValue : public UDialogueK2Node_SwitchDialogueCallbackIntValue
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

UCLASS()
class UDlgK2Node_SwitchDialogueCallbackBoolValue : public UDialogueK2Node_SwitchDialogueCallbackBoolValue
{
	GENERATED_BODY()
	MAKE_NODE_FITERED_OUT()
};

#undef MAKE_NODE_FITERED_OUT
