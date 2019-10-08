// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once
#include "CoreMinimal.h"

#include "Nodes/DlgNode.h"

#include "DlgNode_Selector.generated.h"


UENUM()
enum class EDlgNodeSelectorType : uint8
{
	/** As soon as it is entered it selects its first satisfied child. */
	First		UMETA(DisplayName = "First"),

	/** As soon as it is entered it selects a satisfied child randomly. */
	Random		UMETA(DisplayName = "Random"),
};

/**
 * Node without text. Selector of child depends on the type.
 * It should have at least one (satisfied child), HandleNodeEnter returns false and the Dialogue is terminated otherwise.
 */
UCLASS(BlueprintType)
class DLGSYSTEM_API UDlgNode_Selector : public UDlgNode
{
	GENERATED_BODY()

public:
	UDlgNode_Selector() { bCheckChildrenOnEvaluation = true; }

	/** @return a one line description of an object. */
	FString GetDesc() override
	{
		switch (SelectorType)
		{
		case EDlgNodeSelectorType::First:
			return TEXT("Node without text and as soon as entered it selects its first satisfied child.\n It should have at least one (satisfied child), otherwise the Dialogue is terminated.");
		case EDlgNodeSelectorType::Random:
			return TEXT("Node without text and as soon as entered it selects a satisfied child randomly.\nIt should have at least one (satisfied child), otherwise the Dialogue is terminated.");
		default:
			return TEXT("UNHANDLED");
		}
	}

	// Begin UDlgNode Interface.
	bool HandleNodeEnter(UDlgContextInternal* DlgContext, TSet<const UDlgNode*> NodesEnteredWithThisStep) override;

#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Selector"); }
#endif

	// Begin own functions
	/** Gets the Selector Type */
	EDlgNodeSelectorType GetSelectorType() const { return SelectorType; }

	/** Sets the Selector Type */
	void SetSelectorType(EDlgNodeSelectorType InType) { SelectorType = InType; }

	/** Helper functions to get the names of some properties. Used by the DlgSystemEditor module. */
	static FName GetMemberNameSelectorType() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Selector, SelectorType); }

private:
	/** Defines the type of selector this node represents */
	UPROPERTY(EditAnywhere, Category = DialogueNodeData)
	EDlgNodeSelectorType SelectorType = EDlgNodeSelectorType::First;
};
