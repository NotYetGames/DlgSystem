// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgDialogueParticipantData.generated.h"

struct FDlgCondition;
struct FDlgEvent;
struct FDlgTextArgument;

/** Structure useful to cache all the names used by a participant */
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgParticipantData
{
	GENERATED_USTRUCT_BODY()

public:
	/** Helper functions to fill the data */
	void AddConditionPrimaryData(const FDlgCondition& Condition);
	void AddConditionSecondaryData(const FDlgCondition& Condition);
	void AddEventData(const FDlgEvent& Event);
	void AddTextArgumentData(const FDlgTextArgument& TextArgument);

public:
	/** FName based conditions (aka conditions of type EventCall). */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> Conditions;

	/** FName based events (aka events of type EDlgEventType) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> Events;

	/** Integers used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> IntVariableNames;

	/** Floats used in a Dialogue  */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> FloatVariableNames;

	/** Booleans used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> BoolVariableNames;

	/** Names used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> NameVariableNames;

	/** Class Integers used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> ClassIntVariableNames;

	/** Class Floats used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> ClassFloatVariableNames;

	/** Class Booleans used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> ClassBoolVariableNames;

	/** Class Names used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> ClassNameVariableNames;

	/** Class Texts used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DialogueParticipantData)
	TSet<FName> ClassTextVariableNames;
};
