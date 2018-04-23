// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgDialogueParticipantData.generated.h"


/** Structure useful to cache all the names used by a participant */
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgParticipantData
{
	GENERATED_USTRUCT_BODY()

public:

	/** Helper functions to fill the data */
	void AddConditionPrimaryData(const struct FDlgCondition& Condition);
	void AddConditionSecondaryData(const struct FDlgCondition& Condition);
	void AddEventData(const struct FDlgEvent& Event);
	void AddTextArgumentData(const struct FDlgTextArgument& TextArgument);

public:

	/** FName based conditions (aka conditions of type DlgConditionEventCall). */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> Conditions;

	/** FName based events (aka events of type EDlgEventType) */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> Events;

	/** Integers used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> IntVariableNames;

	/** Floats used in a Dialogue  */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> FloatVariableNames;

	/** Booleans used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> BoolVariableNames;

	/** Names used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> NameVariableNames;

	/** Class Integers used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> ClassIntVariableNames;

	/** Class Floats used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> ClassFloatVariableNames;

	/** Class Booleans used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> ClassBoolVariableNames;

	/** Class Names used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> ClassNameVariableNames;

	/** Class Texts used in a Dialogue */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = DlgParticipantData)
	TSet<FName> ClassTextVariableNames;
};
