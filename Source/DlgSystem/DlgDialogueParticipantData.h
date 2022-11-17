// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgDialogueParticipantData.generated.h"

struct FDlgCondition;
struct FDlgEvent;
struct FDlgTextArgument;

// Structure useful to cache all the names used by a participant
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgParticipantData
{
	GENERATED_USTRUCT_BODY()

public:
	// Helper functions to fill the data
	void AddConditionPrimaryData(const FDlgCondition& Condition);
	void AddConditionSecondaryData(const FDlgCondition& Condition);
	void AddEventData(const FDlgEvent& Event);
	void AddTextArgumentData(const FDlgTextArgument& TextArgument);

public:
	// FName based conditions (aka conditions of type EventCall).
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> Conditions;

	// Custom Conditions UClasses of type UDlgConditionCustom
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<UClass*> CustomConditions;

	// FName based events (aka events of type EDlgEventType)
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> Events;

	// Custom Events UClasses of type UDlgEventCustom
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<UClass*> CustomEvents;

	// Custom Events UClasses of type UDlgTextArgumentCustom
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<UClass*> CustomTextArguments;

	// Integers used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> IntVariableNames;

	// Floats used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> FloatVariableNames;

	// Booleans used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> BoolVariableNames;

	// Names used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> NameVariableNames;

	// Class Integers used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> ClassIntVariableNames;

	// Class Floats used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> ClassFloatVariableNames;

	// Class Booleans used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> ClassBoolVariableNames;

	// Class Names used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> ClassNameVariableNames;

	// Class Texts used in a Dialogue
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Dialogue|Participant")
	TSet<FName> ClassTextVariableNames;
};
