// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Containers/Set.h"

class UDlgDialogue;
class FDlgDataDisplayActorProperties;
class FDlgDataDisplayVariableProperties;

typedef TSharedPtr<FDlgDataDisplayActorProperties> FDlgDataDisplayActorPropertiesPtr;
typedef TSharedPtr<FDlgDataDisplayVariableProperties> FDlgDataDisplayVariablePropertiesPtr;

/** Used as a key for the variable property */
class FDlgDataDisplayVariableProperties
{

public:
	FDlgDataDisplayVariableProperties(const TSet<TWeakObjectPtr<UDlgDialogue>>& InDialogues) : Dialogues(InDialogues) {}
	FDlgDataDisplayVariableProperties(const TSet<TWeakObjectPtr<UDlgDialogue>>&& InDialogues) : Dialogues(InDialogues) {}

	// Dialogues:
	void AddDialogue(TWeakObjectPtr<UDlgDialogue> Dialogue) { Dialogues.Add(Dialogue); }
	const TSet<TWeakObjectPtr<UDlgDialogue>>& GetDialogues() const { return Dialogues; }

protected:
	/** Dialogues that contain this variable property */
	TSet<TWeakObjectPtr<UDlgDialogue>> Dialogues;

	// TODO Value template
};

/** Used as a key for the Actor in the fast lookup table. */
class FDlgDataDisplayActorProperties
{
	typedef FDlgDataDisplayActorProperties Self;

public:
	FDlgDataDisplayActorProperties(const TSet<TWeakObjectPtr<UDlgDialogue>>& InDialogues) : Dialogues(InDialogues) {}
	FDlgDataDisplayActorProperties(const TSet<TWeakObjectPtr<UDlgDialogue>>&& InDialogues) : Dialogues(InDialogues) {}

	/** Add Dialogue to current set. */
	void AddDialogue(TWeakObjectPtr<UDlgDialogue> Dialogue) { Dialogues.Add(Dialogue); }

	/** Returns the EventName Property */
	FDlgDataDisplayVariablePropertiesPtr AddDialogueToEvent(const FName& EventName, TWeakObjectPtr<UDlgDialogue> Dialogue)
	{
		return AddDialogueToVariable<FDlgDataDisplayVariableProperties, FDlgDataDisplayVariablePropertiesPtr>(&Events, EventName, Dialogue);
	}

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
	 * Dialogues that contain the participant referenced by this Actor
	 */
	TSet<TWeakObjectPtr<UDlgDialogue>> Dialogues;

	/**
	 * Events that belong to this participant
	 * Key: Event Name
	 * Value: The Properties of this event.
	 */
	TMap<FName, FDlgDataDisplayVariablePropertiesPtr> Events;
};
