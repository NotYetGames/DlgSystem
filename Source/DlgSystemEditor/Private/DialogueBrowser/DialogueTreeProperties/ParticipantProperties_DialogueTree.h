// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once
#include "CoreMinimal.h"

#include "VariableProperties_DialogueTree.h"

class UDlgDialogue;
class FParticipantProperties_DialogueTree;

typedef TSharedPtr<FParticipantProperties_DialogueTree> FDlgTreeParticipantPropertiesPtr;

/** Used as a key in the fast lookup table. */
class FParticipantProperties_DialogueTree
{
	typedef FParticipantProperties_DialogueTree Self;

public:
	FParticipantProperties_DialogueTree(const TSet<TWeakObjectPtr<UDlgDialogue>>& InDialogues) : Dialogues(InDialogues) {}
	FParticipantProperties_DialogueTree(const TSet<TWeakObjectPtr<UDlgDialogue>>&& InDialogues) : Dialogues(InDialogues) {}

	/** Does this participant has any int/float/bool variables */
	bool HasVariables() const
	{
		return Integers.Num() > 0 || Floats.Num() > 0 || Bools.Num() > 0;
	}

	/** Sorts all the properties it can */
	void Sort();

	/** Add Dialogue to current set. */
	void AddDialogue(TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		Dialogues.Add(Dialogue);
	}

	/** Returns the EventName Property */
	FDlgTreeVariablePropertiesPtr AddDialogueToEvent(const FName& EventName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable<FVariableProperties_DialogueTree, FDlgTreeVariablePropertiesPtr>(&Events, EventName, Dialogue);
	}

	/** Returns the ConditionName Property */
	FDlgTreeVariablePropertiesPtr AddDialogueToCondition(const FName& ConditionName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable<FVariableProperties_DialogueTree, FDlgTreeVariablePropertiesPtr>(&Conditions, ConditionName, Dialogue);
	}

	/** Returns the IntName Property */
	FDlgTreeVariablePropertiesPtr AddDialogueToIntVariable(const FName& IntVariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable<FVariableProperties_DialogueTree, FDlgTreeVariablePropertiesPtr>(&Integers, IntVariableName, Dialogue);
	}

	/** Returns the FloatName Property */
	FDlgTreeVariablePropertiesPtr AddDialogueToFloatVariable(const FName& FloatVariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable<FVariableProperties_DialogueTree, FDlgTreeVariablePropertiesPtr>(&Floats, FloatVariableName, Dialogue);
	}

	/** Returns the BoolName Property */
	FDlgTreeVariablePropertiesPtr AddDialogueToBoolVariable(const FName& BoolVariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable<FVariableProperties_DialogueTree, FDlgTreeVariablePropertiesPtr>(&Bools, BoolVariableName, Dialogue);
	}

	const TSet<TWeakObjectPtr<UDlgDialogue>>& GetDialogues() const { return Dialogues; }
	const TMap<FName, FDlgTreeVariablePropertiesPtr>& GetEvents() const { return Events; }
	const TMap<FName, FDlgTreeVariablePropertiesPtr>& GetConditions() const { return Conditions; }
	const TMap<FName, FDlgTreeVariablePropertiesPtr>& GetIntegers() const { return Integers; }
	const TMap<FName, FDlgTreeVariablePropertiesPtr>& GetFloats() const { return Floats; }
	const TMap<FName, FDlgTreeVariablePropertiesPtr>& GetBools() const { return Bools; }

private:
	template<typename VariablePropertyType, typename VariablePropertyTypePtr>
	VariablePropertyTypePtr AddDialogueToVariable(TMap<FName, VariablePropertyTypePtr>* VariableMap,
												  const FName& VariableName,
												 TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		VariablePropertyTypePtr* VariablePropsPtr = VariableMap->Find(VariableName);
		VariablePropertyTypePtr VariableProps;
		if (VariablePropsPtr == nullptr)
		{
			// variable does not exist for participant, create it
			VariableProps = MakeShareable(new VariablePropertyType({Dialogue}));
			VariableMap->Add(VariableName, VariableProps);
		}
		else
		{
			// exists
			VariableProps = *VariablePropsPtr;
			VariableProps->AddDialogue(Dialogue);
		}

		return VariableProps;
	}

private:
	/**
	 * Dialogues that contain this participant
	 * NOTE: can't convert it into a map with the key as an FName becaus we can expect duplicate dialogues
	 */
	TSet<TWeakObjectPtr<UDlgDialogue>> Dialogues;

	/**
	 * Events that belong to this participant
	 * Key: Event Name
	 * Value: The properties of this event
	 */
	TMap<FName, FDlgTreeVariablePropertiesPtr> Events;

	/**
	 * Conditions that belong to this participant
	 * Key: Condition Name
	 * Value: The properties of this condition
	 */
	TMap<FName, FDlgTreeVariablePropertiesPtr> Conditions;

	/**
	 * Int variables that belong to this participant
	 * Key: Int Variable Name
	 * Value: The properties of this int variable
	 */
	TMap<FName, FDlgTreeVariablePropertiesPtr> Integers;

	/**
	 * Float variables that belong to this participant
	 * Key: Float Variable Name
	 * Value: The properties of this float variable
	 */
	TMap<FName, FDlgTreeVariablePropertiesPtr> Floats;

	/**
	 * Bool variables that belong to this participant
	 * Key: Bool Variable Name
	 * Value: The properties of this bool variable
	 */
	TMap<FName, FDlgTreeVariablePropertiesPtr> Bools;
};
