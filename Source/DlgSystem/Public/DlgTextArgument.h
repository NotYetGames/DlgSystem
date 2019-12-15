// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgTextArgument.generated.h"

class IDlgDialogueParticipant;
class UDlgContextInternal;


/**
 *  Argument type, which defines both the type of the argument and the way the system will acquire the value
 */
UENUM()
enum class EDlgTextArgumentType : uint8
{
	DisplayName = 0	UMETA(DisplayName = "Participant Display Name"),
	Gender			UMETA(DisplayName = "Participant Gender"),

	DialogueInt		UMETA(DisplayName = "Dialogue Int Variable"),
	ClassInt		UMETA(DisplayName = "Class Int Variable"),

	DialogueFloat	UMETA(DisplayName = "Dialogue Float Variable"),
	ClassFloat		UMETA(DisplayName = "Class Float Variable"),

	ClassText		UMETA(DisplayName = "Class Text Variable")
};

/**
 * An argument is a variable to extend node texts with dynamic values runtime
 * It can be inserted to the FText, the same way FText::Format works
 * See: https://docs.unrealengine.com/en-us/Gameplay/Localization/Formatting
 */
USTRUCT(Blueprintable, BlueprintType)
struct DLGSYSTEM_API FDlgTextArgument
{
	GENERATED_USTRUCT_BODY()

public:

	bool operator==(const FDlgTextArgument& Other) const;

	/** Construct the argument for usage in FText::Format */
	FFormatArgumentValue ConstructFormatArgumentValue(const UDlgContextInternal* DlgContext, FName NodeOwner) const;

	/** Helper method to update the array InOutArgumentArray with the new arguments from Text. */
	static void UpdateTextArgumentArray(const FText& Text, TArray<FDlgTextArgument>& InOutArgumentArray);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DialogueTextArgument)
	FString DisplayString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueTextArgument)
	EDlgTextArgumentType Type = EDlgTextArgumentType::DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueTextArgument)
	FName ParticipantName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueTextArgument)
	FName VariableName;

public:

	// Operator overload for serialization
	friend FArchive& operator<<(FArchive &Ar, FDlgTextArgument& DlgCondition);
};


FORCEINLINE bool FDlgTextArgument::operator==(const FDlgTextArgument& Other) const
{
	return	DisplayString == Other.DisplayString &&
			Type == Other.Type &&
			ParticipantName == Other.ParticipantName &&
			VariableName == Other.VariableName;
}
