// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "K2Node.h"
#include "EdGraphSchema_K2.h"

#include "DialogueK2Node_Select.generated.h"


UENUM()
enum class EDlgVariableType : uint8
{
	Float			UMETA(DisplayName = "Dialogue Float Variable"),
	Int				UMETA(DisplayName = "Dialogue Int Variable"),
	Name			UMETA(DisplayName = "Dialogue Name Variable"),
	SpeakerState	UMETA(DisplayName = "Dialogue Speaker State"),
};

/**
 * Select BlueprintNode Based on UK2Node_Select
 */
UCLASS(MinimalAPI, Meta=(Keywords = "Ternary If"))
class UDialogueK2Node_Select : public UK2Node
{
	GENERATED_BODY()

public:
	UDialogueK2Node_Select(const FObjectInitializer& ObjectInitializer);

	//~ Begin UEdGraphNode Interface
	void AllocateDefaultPins() override;
	void PinTypeChanged(UEdGraphPin* Pin) override;
	FText GetTooltipText() const override;
	void NodeConnectionListChanged() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	bool IsNodeSafeToIgnore() const override { return true; }
	bool IsNodePure() const override { return true; }
	class FNodeHandlingFunctor* CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const override;
	bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	FText GetMenuCategory() const override;
	int32 GetNodeRefreshPriority() const override { return EBaseNodeRefreshPriority::Normal; }
	void PostReconstructNode() override;
	//~ End UK2Node Interface

	// Begin own functions
	/** Gets the return value pin */
	UEdGraphPin* GetReturnValuePin() const
	{
		check(Pins.IsValidIndex(INDEX_PIN_Return) && Pins[INDEX_PIN_Return]->PinName == UEdGraphSchema_K2::PN_ReturnValue)
		return Pins[INDEX_PIN_Return];
	}

	// The return pin is alright, because it is the only output pin
	/** Gets the condition pin */
	UEdGraphPin* GetVariableNamePin() const
	{
		check(Pins.IsValidIndex(INDEX_PIN_VariableName) && Pins[INDEX_PIN_VariableName]->PinName == PIN_VariableName)
		return Pins[INDEX_PIN_VariableName];
	}

	/** Gets the default value pin */
	UEdGraphPin* GetDefaultValuePin() const
	{
		check(Pins.IsValidIndex(INDEX_PIN_Default) && Pins[INDEX_PIN_Default]->PinName == PIN_DefaultValue)
		return Pins[INDEX_PIN_Default];
	}

	/** Returns a list of pins that represent the selectable options */
	const TArray<UEdGraphPin*> GetOptionPins() const
	{
		// The end of the array is all option pins
		TArray<UEdGraphPin*> OptionPins;
		for (int32 PinIndex = INDEX_PIN_OPTIONS_START, PinsNum = Pins.Num(); PinIndex < PinsNum; PinIndex++)
		{
			// check(Pins[PinIndex]->PinType.PinCategory == VariablePinType);
			OptionPins.Add(Pins[PinIndex]);
		}

		return OptionPins;
	}

	/** Gets the function that compares FNames (EqualEqual_NameName function) */
	static UFunction* GetConditionalFunction();

	/** Gets the name and class of the PrintString function */
	static void GetPrintStringFunction(FName& FunctionName, UClass** FunctionClass);

	/** Refreshes the PinNames array */
	bool RefreshPinNames();

private:
	/** Refreshes the VariablePinType  */
	void RefreshVariablePinType()
	{
		switch (VariableType)
		{
		case EDlgVariableType::Float:
			VariablePinType = UEdGraphSchema_K2::PC_Float;
			break;
		case EDlgVariableType::Int:
			VariablePinType = UEdGraphSchema_K2::PC_Int;
			break;
		case EDlgVariableType::Name:
			VariablePinType = UEdGraphSchema_K2::PC_Name;
			break;
		case EDlgVariableType::SpeakerState:
			VariablePinType = UEdGraphSchema_K2::PC_Wildcard;
			break;
		default:
			unimplemented();
		}
	}

public:
	// Constants for the location of the input/output pins in the Pins array
	static constexpr int32 INDEX_PIN_Return = 0;
	static constexpr int32 INDEX_PIN_VariableName = 1;
	static constexpr int32 INDEX_PIN_Default = 2;
	static constexpr int32 INDEX_PIN_OPTIONS_START = 3;

protected:
	/** List of the current entries (Pin Names) based on owner dialogue name and on all dialogues */
	UPROPERTY()
	TArray<FName> PinNames;

	UPROPERTY()
	EDlgVariableType VariableType;

	// The pin type of this select
	UPROPERTY()
	FName VariablePinType;

	/** Whether we need to reconstruct the node after the pins have changed */
	UPROPERTY(Transient)
	bool bReconstructNode;

	// Constants.
	static const FName PIN_VariableName; // index
	static const FName PIN_DefaultValue;
};

/**
 *  Float variant
 */
UCLASS(MinimalAPI, Meta=(Keywords = "Ternary If"))
class UDialogueK2Node_SelectFloat : public UDialogueK2Node_Select
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SelectFloat(const FObjectInitializer& ObjectInitializer);

	//~ Begin UEdGraphNode Interface
	FText GetTooltipText() const override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
};

/**
 *  Name variant
 */
UCLASS(MinimalAPI, Meta=(Keywords = "Ternary If"))
class UDialogueK2Node_SelectName : public UDialogueK2Node_Select
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SelectName(const FObjectInitializer& ObjectInitializer);

	//~ Begin UEdGraphNode Interface
	FText GetTooltipText() const override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
};

/**
 *  SpeakerState variant
 */
UCLASS(MinimalAPI, Meta=(Keywords = "Ternary If"))
class UDialogueK2Node_SelectOnSpeakerState : public UDialogueK2Node_Select
{
	GENERATED_BODY()

public:
	UDialogueK2Node_SelectOnSpeakerState(const FObjectInitializer& ObjectInitializer);

	//~ Begin UEdGraphNode Interface
	FText GetTooltipText() const override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
};
