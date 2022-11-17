// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DialogueK2Node_SwitchDialogueCallback.h"

#include "DialogueK2Node_SwitchDialogueCallbackVariants.generated.h"

/**
 *   Subclasses for quicker access in blueprint
 *	 They just set and hide the type enum property of the parent
 */

UCLASS()
class UDialogueK2Node_SwitchDialogueCallbackEvent : public UDialogueK2Node_SwitchDialogueCallback
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SwitchDialogueCallbackEvent(const FObjectInitializer& ObjectInitializer);

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
};

UCLASS()
class UDialogueK2Node_SwitchDialogueCallbackCondition : public UDialogueK2Node_SwitchDialogueCallback
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SwitchDialogueCallbackCondition(const FObjectInitializer& ObjectInitializer);

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
};

UCLASS()
class UDialogueK2Node_SwitchDialogueCallbackFloatValue : public UDialogueK2Node_SwitchDialogueCallback
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SwitchDialogueCallbackFloatValue(const FObjectInitializer& ObjectInitializer);

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
};

UCLASS()
class UDialogueK2Node_SwitchDialogueCallbackIntValue : public UDialogueK2Node_SwitchDialogueCallback
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SwitchDialogueCallbackIntValue(const FObjectInitializer& ObjectInitializer);

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
};

UCLASS()
class UDialogueK2Node_SwitchDialogueCallbackBoolValue : public UDialogueK2Node_SwitchDialogueCallback
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SwitchDialogueCallbackBoolValue(const FObjectInitializer& ObjectInitializer);

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
};

UCLASS()
class UDialogueK2Node_SwitchDialogueCallbackNameValue : public UDialogueK2Node_SwitchDialogueCallback
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SwitchDialogueCallbackNameValue(const FObjectInitializer& ObjectInitializer);

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
};
