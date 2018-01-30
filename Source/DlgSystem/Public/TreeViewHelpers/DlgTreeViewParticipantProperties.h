// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include <type_traits>
#include "CoreMinimal.h"

#include "DlgManager.h"
#include "DlgTreeViewVariableProperties.h"

/** Structure that holds the common properties of a Participant (where it appears and stuff like this) in the STreeView. */
template <class VariablePropertyType>
class FDlgTreeViewParticipantProperties
{
	// Compile time Typecheck if subclass
	typedef typename std::enable_if<std::is_base_of<FDlgTreeViewVariableProperties, VariablePropertyType>::value>::type check;

public:
	FDlgTreeViewParticipantProperties(const TSet<TWeakObjectPtr<UDlgDialogue>>& InDialogues) : Dialogues(InDialogues) {}
	FDlgTreeViewParticipantProperties(const TSet<TWeakObjectPtr<UDlgDialogue>>&& InDialogues) : Dialogues(InDialogues) {}

	/** Sorts all the properties it can */
	void Sort()
	{
		Dialogues.Sort(PredicateSortDialogueWeakPtrAlphabeticallyAscending);
		for (const auto& Pair : Events)
		{
			Pair.Value->Sort();
		}

		UDlgManager::SortDefault(Events);
		UDlgManager::SortDefault(Conditions);
		UDlgManager::SortDefault(Integers);
		UDlgManager::SortDefault(Floats);
		UDlgManager::SortDefault(Bools);
		UDlgManager::SortDefault(FNames);
	}

	/** Does this participant has any int/float/bool variables */
	bool HasVariables() const
	{
		return Integers.Num() > 0 || Floats.Num() > 0 || Bools.Num() > 0 || FNames.Num() > 0;
	}

	// Setters

	/** Add Dialogue that containt this participant. */
	void AddDialogue(TWeakObjectPtr<UDlgDialogue> Dialogue) { Dialogues.Add(Dialogue); }

	/** Returns the EventName Property */
	TSharedPtr<VariablePropertyType> AddDialogueToEvent(const FName& EventName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Events, EventName, Dialogue);
	}

	/** Returns the ConditionName Property */
	TSharedPtr<VariablePropertyType> AddDialogueToCondition(const FName& ConditionName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Conditions, ConditionName, Dialogue);
	}

	/** Returns the IntName Property */
	TSharedPtr<VariablePropertyType> AddDialogueToIntVariable(const FName& IntVariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Integers, IntVariableName, Dialogue);
	}

	/** Returns the FloatName Property */
	TSharedPtr<VariablePropertyType> AddDialogueToFloatVariable(const FName& FloatVariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Floats, FloatVariableName, Dialogue);
	}

	/** Returns the BoolName Property */
	TSharedPtr<VariablePropertyType> AddDialogueToBoolVariable(const FName& BoolVariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Bools, BoolVariableName, Dialogue);
	}

	/** Returns the FName Property */
	TSharedPtr<VariablePropertyType> AddDialogueToFNameVariable(const FName& FNameVariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&FNames, FNameVariableName, Dialogue);
	}

	// Getters
	const TSet<TWeakObjectPtr<UDlgDialogue>>& GetDialogues() const { return Dialogues; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetEvents() const { return Events; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetConditions() const { return Conditions; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetIntegers() const { return Integers; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetFloats() const { return Floats; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetBools() const { return Bools; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetFNames() const { return FNames; }

protected:
	TSharedPtr<VariablePropertyType> AddDialogueToVariable(TMap<FName, TSharedPtr<VariablePropertyType>>* VariableMap,
		const FName& VariableName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		TSharedPtr<VariablePropertyType>* VariablePropsPtr = VariableMap->Find(VariableName);
		TSharedPtr<VariablePropertyType> VariableProps;
		if (VariablePropsPtr == nullptr)
		{
			// Variable does not exist for participant, create it
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

protected:
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
	TMap<FName, TSharedPtr<VariablePropertyType>> Events;

	/**
	 * Conditions that belong to this participant
	 * Key: Condition Name
	 * Value: The properties of this condition
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> Conditions;

	/**
	 * Int variables that belong to this participant
	 * Key: Int Variable Name
	 * Value: The properties of this int variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> Integers;

	/**
	 * Float variables that belong to this participant
	 * Key: Float Variable Name
	 * Value: The properties of this float variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> Floats;

	/**
	 * Bool variables that belong to this participant
	 * Key: Bool Variable Name
	 * Value: The properties of this bool variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> Bools;

	/**
	 * FNames variables that belong to this participant
	 * Key: FName Variable Name
	 * Value: The properties of this FName variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> FNames;
};
