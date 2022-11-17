// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "K2Node_Switch.h"

#include "DialogueK2Node_SwitchDialogueCallback.generated.h"


UENUM()
enum class EDlgDialogueCallback : uint8
{
	/** Normal dialogue event during the conversation */
	Event UMETA(DisplayName = "Dialogue Event"),
	/** Function call to check a condition */
	Condition UMETA(DisplayName = "Dialogue Condition"),

	FloatValue UMETA(DisplayName = "Condition asking for a float value"),

	IntValue UMETA(DisplayName = "Condition asking for an int value"),

	BoolValue UMETA(DisplayName = "Condition asking for a bool value"),

	NameValue UMETA(DisplayName = "Condition asking for a name value"),
};

/**
 *
 */
UCLASS()
class UDialogueK2Node_SwitchDialogueCallback : public UK2Node_Switch
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SwitchDialogueCallback(const FObjectInitializer& ObjectInitializer);

	// UEdGraphNode interface
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
	// End of UEdGraphNode interface

	// UK2Node interface
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	FText GetMenuCategory() const override;
	bool ShouldShowNodeProperties() const override { return true; }
	// End of UK2Node interface

	// UK2Node_Switch Interface
	void AddPinToSwitchNode() override;
	FName GetUniquePinName() override;
	FName GetPinNameGivenIndex(int32 Index) const override;
	FEdGraphPinType GetPinType() const override;
	void RemovePinFromSwitchNode(UEdGraphPin* TargetPin) override;
	bool CanRemoveExecutionPin(UEdGraphPin* TargetPin) const override;
	// End of UK2Node_Switch Interface

protected:
	// UK2Node_Switch Interface
	void CreateCasePins() override;
	void CreateSelectionPin() override;
	void CreateFunctionPin() override;
	void RemovePin(UEdGraphPin* TargetPin) override {}
	// End of UK2Node_Switch Interface

	// Begin own functions
	// updates DialoguePinNames, return value is true if it is changed
	bool RefreshPinNames();

protected:
	UPROPERTY()
	TArray<FName> DialoguePinNames;

	UPROPERTY()
	EDlgDialogueCallback CallbackType;
};
