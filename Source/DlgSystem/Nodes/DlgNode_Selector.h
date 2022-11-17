// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

#include "DlgSystem/Nodes/DlgNode.h"

#include "DlgNode_Selector.generated.h"


UENUM(BlueprintType)
enum class EDlgNodeSelectorType : uint8
{
	// As soon as it is entered it selects its first satisfied child.
	First		UMETA(DisplayName = "First"),

	// As soon as it is entered it selects a satisfied child randomly.
	Random		UMETA(DisplayName = "Random"),
};

/**
 * Node without text. Selector of child depends on the type.
 * It should have at least one (satisfied child), HandleNodeEnter returns false and the Dialogue is terminated otherwise.
 */
UCLASS(BlueprintType, ClassGroup = "Dialogue")
class DLGSYSTEM_API UDlgNode_Selector : public UDlgNode
{
	GENERATED_BODY()

public:
	UDlgNode_Selector() { bCheckChildrenOnEvaluation = true; }

	const FText& GetNodeText() const override;

	// @return a one line description of an object.
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

	//
	// Begin UDlgNode Interface.
	//

	bool HandleNodeEnter(UDlgContext& Context, TSet<const UDlgNode*> NodesEnteredWithThisStep) override;

#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Selector"); }
#endif

	//
	// Begin own functions
	//

	// Gets the Selector Type
	UFUNCTION(BlueprintPure, Category = "Dialogue|Node")
	EDlgNodeSelectorType GetSelectorType() const { return SelectorType; }

	// Sets the Selector Type
	void SetSelectorType(EDlgNodeSelectorType InType) { SelectorType = InType; }

	// Helper functions to get the names of some properties. Used by the DlgSystemEditor module.
	static FName GetMemberNameSelectorType() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Selector, SelectorType); }
	static FName GetMemberNameAvoidPickingSameOptionTwiceInARow() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Selector, bAvoidPickingSameOptionTwiceInARow); }
	static FName GetMemberNameCycleThroughSatisfiedOptionsWithoutRepetition() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Selector, bCycleThroughSatisfiedOptionsWithoutRepetition); }

protected:

	int32 GetRandomChildNodeIndex(UDlgContext& Context);

protected:
	// Defines the type of selector this node represents
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node")
	EDlgNodeSelectorType SelectorType = EDlgNodeSelectorType::First;

	// Modifies the way EDlgNodeSelectorType::Random works.
	// Ensures that an option is not picked twice before any other option is,
	// unless it is not possible because of the node setup/conditions
	UPROPERTY(EditAnywhere, meta = (EditCondition = "SelectorType == EDlgNodeSelectorType::Random", EditConditionHides), Category = "Dialogue|Node")
	bool bAvoidPickingSameOptionTwiceInARow = false;

	// Only after each satisfied option is picked once can an option be picked again
	// Still allows repetition if bAvoidPickingSameOptionTwiceInARow is not set to true,
	// e.g. for options {A, B, C} A-B-C-C-A-B-B... is a valid series of choices
	UPROPERTY(EditAnywhere, meta = (EditCondition = "SelectorType == EDlgNodeSelectorType::Random", EditConditionHides), Category = "Dialogue|Node")
	bool bCycleThroughSatisfiedOptionsWithoutRepetition = false;


	UPROPERTY(Transient)
	mutable FText DynamicDisplayText;
};
