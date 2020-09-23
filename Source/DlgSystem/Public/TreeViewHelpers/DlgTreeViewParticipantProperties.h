// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include <type_traits>
#include "CoreMinimal.h"

#include "DlgManager.h"
#include "TreeViewHelpers/DlgTreeViewVariableProperties.h"
#include "TreeViewHelpers/DlgTreeViewHelper.h"

/** Structure that holds the common properties of a Participant (where it appears and stuff like this) in the STreeView. */
template <class VariablePropertyType>
class FDlgTreeViewParticipantProperties
{
	// Compile time Typecheck if subclass
	typedef typename std::enable_if<std::is_base_of<FDlgTreeViewVariableProperties, VariablePropertyType>::value>::type check;

public:
	FDlgTreeViewParticipantProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>& InDialogues) : Dialogues(InDialogues) {}
	FDlgTreeViewParticipantProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>&& InDialogues) : Dialogues(InDialogues) {}

	/** Sorts all the properties it can */
	void Sort()
	{
		Dialogues.Sort(FDlgTreeViewHelper::PredicateSortDialogueWeakPtrAlphabeticallyAscending);
		for (const auto& Pair : Events)
		{
			Pair.Value->Sort();
		}

		FDlgHelper::SortDefault(Events);
		FDlgHelper::SortDefault(Conditions);
		FDlgHelper::SortDefault(Integers);
		FDlgHelper::SortDefault(Floats);
		FDlgHelper::SortDefault(Bools);
		FDlgHelper::SortDefault(FNames);
		FDlgHelper::SortDefault(ClassIntegers);
		FDlgHelper::SortDefault(ClassFloats);
		FDlgHelper::SortDefault(ClassBools);
		FDlgHelper::SortDefault(ClassFNames);
		FDlgHelper::SortDefault(ClassFTexts);
	}

	// Setters

	// Add Dialogue that containt this participant.
	void AddDialogue(TWeakObjectPtr<const UDlgDialogue> Dialogue) { Dialogues.Add(Dialogue); }

	// Returns the EventName Property
	TSharedPtr<VariablePropertyType> AddDialogueToEvent(FName EventName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Events, EventName, Dialogue);
	}

	// Returns the EventClass Property
	TSharedPtr<VariablePropertyType> AddDialogueToCustomEvent(UClass* EventClass, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable<UClass*>(&CustomEvents, EventClass, Dialogue);
	}

	// Returns the ConditionName Property
	TSharedPtr<VariablePropertyType> AddDialogueToCondition(FName ConditionName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Conditions, ConditionName, Dialogue);
	}

	// Returns the IntName Property
	TSharedPtr<VariablePropertyType> AddDialogueToIntVariable(FName IntVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Integers, IntVariableName, Dialogue);
	}

	// Returns the FloatName Property
	TSharedPtr<VariablePropertyType> AddDialogueToFloatVariable(FName FloatVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Floats, FloatVariableName, Dialogue);
	}

	// Returns the BoolName Property
	TSharedPtr<VariablePropertyType> AddDialogueToBoolVariable(FName BoolVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&Bools, BoolVariableName, Dialogue);
	}

	// Returns the FName Property
	TSharedPtr<VariablePropertyType> AddDialogueToFNameVariable(FName FNameVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&FNames, FNameVariableName, Dialogue);
	}

	// Returns the IntName Property
	TSharedPtr<VariablePropertyType> AddDialogueToClassIntVariable(FName IntVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&ClassIntegers, IntVariableName, Dialogue);
	}

	// Returns the FloatName Property
	TSharedPtr<VariablePropertyType> AddDialogueToClassFloatVariable(FName FloatVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&ClassFloats, FloatVariableName, Dialogue);
	}

	// Returns the BoolName Property
	TSharedPtr<VariablePropertyType> AddDialogueToClassBoolVariable(FName BoolVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&ClassBools, BoolVariableName, Dialogue);
	}

	// Returns the FName Property
	TSharedPtr<VariablePropertyType> AddDialogueToClassFNameVariable(FName FNameVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&ClassFNames, FNameVariableName, Dialogue);
	}

	// Returns the FText Property *
	TSharedPtr<VariablePropertyType> AddDialogueToClassFTextVariable(FName FTextVariableName, TWeakObjectPtr<const UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable(&ClassFTexts, FTextVariableName, Dialogue);
	}

	// Getters
	const TSet<TWeakObjectPtr<const UDlgDialogue>>& GetDialogues() const { return Dialogues; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetEvents() const { return Events; }
	const TMap<UClass*, TSharedPtr<VariablePropertyType>>& GetCustomEvents() const { return CustomEvents; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetConditions() const { return Conditions; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetIntegers() const { return Integers; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetFloats() const { return Floats; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetBools() const { return Bools; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetFNames() const { return FNames; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetClassIntegers() const { return ClassIntegers; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetClassFloats() const { return ClassFloats; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetClassBools() const { return ClassBools; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetClassFNames() const { return ClassFNames; }
	const TMap<FName, TSharedPtr<VariablePropertyType>>& GetClassFTexts() const { return ClassFTexts; }

	/** Does this participant has any dialogue values of the basic type */
	bool HasDialogueValues() const
	{
		return Integers.Num() > 0 || Floats.Num() > 0 || Bools.Num() > 0 || FNames.Num() > 0;
	}

	/** Does this participant has any variables that belong to the UClass of the participant */
	bool HasClassVariables() const
	{
		return ClassIntegers.Num() > 0 || ClassFloats.Num() > 0 || ClassBools.Num() > 0 || ClassFNames.Num() > 0 || ClassFTexts.Num() > 0;
	}

	bool HasDialogues() const { return Dialogues.Num() > 0; }
	bool HasEvents() const { return Events.Num() > 0; }
	bool HasCustomEvents() const { return CustomEvents.Num() > 0; }
	bool HasConditions() const { return Conditions.Num() > 0; }
	bool HasIntegers() const { return Integers.Num() > 0; }
	bool HasFloats() const { return Floats.Num() > 0; }
	bool HasBools() const { return Bools.Num() > 0; }
	bool HasFNames() const { return FNames.Num() > 0; }
	bool HasClassIntegers() const { return ClassIntegers.Num() > 0; }
	bool HasClassFloats() const { return ClassFloats.Num() > 0; }
	bool HasClassBools() const { return ClassBools.Num() > 0; }
	bool HasClassFNames() const { return ClassFNames.Num() > 0; }
	bool HasClassFTexts() const { return ClassFTexts.Num() > 0; }

protected:
	template <typename KeyType>
	TSharedPtr<VariablePropertyType> AddDialogueToVariable(
		TMap<KeyType, TSharedPtr<VariablePropertyType>>* VariableMap,
		KeyType VariableKeyValue,
		TWeakObjectPtr<const UDlgDialogue> Dialogue
	)
	{
		TSharedPtr<VariablePropertyType>* VariablePropsPtr = VariableMap->Find(VariableKeyValue);
		TSharedPtr<VariablePropertyType> VariableProps;
		if (VariablePropsPtr == nullptr)
		{
			// Variable does not exist for participant, create it
			const TSet<TWeakObjectPtr<const UDlgDialogue>> SetArgument{Dialogue};
			VariableProps = MakeShared<VariablePropertyType>(SetArgument);
			VariableMap->Add(VariableKeyValue, VariableProps);
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
	TSet<TWeakObjectPtr<const UDlgDialogue>> Dialogues;

	/**
	 * Events that belong to this participant
	 * Key: Event Name
	 * Value: The properties of this event
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> Events;

	/**
	 * Custom Events that belong to this participant
	 * Key: Custom Event Class
	 * Value: The properties of this event
	 */
	TMap<UClass*, TSharedPtr<VariablePropertyType>> CustomEvents;

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

	/**
	 * Int variables that belong to the UClass of this participant
	 * Key: Int Variable Name
	 * Value: The properties of this int variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> ClassIntegers;

	/**
	 * Float variables that belong to the UClass of this participant
	 * Key: Float Variable Name
	 * Value: The properties of this float variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> ClassFloats;

	/**
	 * Bool variables that belong to the UClass of this participant
	 * Key: Bool Variable Name
	 * Value: The properties of this bool variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> ClassBools;

	/**
	 * FNames variables that belong to the UClass of this participant
	 * Key: FName Variable Name
	 * Value: The properties of this FName variable
	 */
	TMap<FName, TSharedPtr<VariablePropertyType>> ClassFNames;

	/**
	* FText variables that belong to the UClass of this participant
	* Key: FText Variable Name
	* Value: The properties of this FText variable
	*/
	TMap<FName, TSharedPtr<VariablePropertyType>> ClassFTexts;
};
