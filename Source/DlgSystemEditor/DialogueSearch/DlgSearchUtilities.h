// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "DlgSystem/DlgEvent.h"
#include "DlgSystem/DlgCondition.h"
#include "DlgSystem/DlgTextArgument.h"

//////////////////////////////////////////////////////////////////////////
// FDialogueESearchUtilities

class UDlgDialogue;
class UDialogueGraphNode;
struct FDlgNode;
class UEdGraph;
class UDialogueGraphNode_Base;
class UDialogueGraphNode_Edge;

// Represents the found result of the search functions.
struct DLGSYSTEMEDITOR_API FDlgSearchFoundResult
{
private:
	typedef FDlgSearchFoundResult Self;

public:
	static TSharedPtr<FDlgSearchFoundResult> Make() { return MakeShared<Self>(); }

public:
	// Nodes that satisfy the search result.
	TArray<TWeakObjectPtr<const UDialogueGraphNode>> GraphNodes;

	// Edges that satisfy the search result.
	TArray<TWeakObjectPtr<const UDialogueGraphNode_Edge>> EdgeNodes;
};

/**
 * Utilities for search.
 * @brief The FDialogueSearchUtilities class
 */
class DLGSYSTEMEDITOR_API FDlgSearchUtilities
{
public:

	// Gets all the graph nodes that contain the specified EventName (of the EventType Event)
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForEventEventName(FName EventName, const UDlgDialogue* Dialogue);

	// Gets all the graph nodes that contain the custom Event EventClass
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForCustomEvent(const UClass* EventClass, const UDlgDialogue* Dialogue);


	/**
	 * Gets all the graph nodes that contain the specified ConditionName (of the ConditionType EventCall)
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForConditionEventCallName(FName ConditionName, const UDlgDialogue* Dialogue);

	/**
	 * Gets all the graph nodes that contain the specified IntVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForIntVariableName(FName IntVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDlgSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(
			IntVariableName,
			Dialogue,
			EDlgEventType::ModifyInt,
			EDlgConditionType::IntCall
		);
		GetGraphNodesForTextArgumentVariable(IntVariableName, Dialogue, EDlgTextArgumentType::DialogueInt, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified FloatVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForFloatVariableName(FName FloatVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDlgSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(
			FloatVariableName,
			Dialogue,
			EDlgEventType::ModifyFloat,
			EDlgConditionType::FloatCall
		);
		GetGraphNodesForTextArgumentVariable(FloatVariableName, Dialogue, EDlgTextArgumentType::DialogueInt, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified BoolVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForBoolVariableName(FName BoolVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(
			BoolVariableName,
			Dialogue,
			EDlgEventType::ModifyBool,
			EDlgConditionType::BoolCall
		);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForFNameVariableName(FName FNameVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(
			FNameVariableName,
			Dialogue,
			EDlgEventType::ModifyName,
			EDlgConditionType::NameCall
		);
	}

	/**
	 * Gets all the graph nodes that contain the specified IntVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForClassIntVariableName(FName IntVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDlgSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(
			IntVariableName,
			Dialogue,
			EDlgEventType::ModifyClassIntVariable,
			EDlgConditionType::ClassIntVariable
		);
		GetGraphNodesForTextArgumentVariable(IntVariableName, Dialogue, EDlgTextArgumentType::ClassInt, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified FloatVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForClassFloatVariableName(FName FloatVariableName, const UDlgDialogue* Dialogue)
	{

		TSharedPtr<FDlgSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(
			FloatVariableName,
			Dialogue,
			EDlgEventType::ModifyClassFloatVariable,
			EDlgConditionType::ClassFloatVariable
		);
		GetGraphNodesForTextArgumentVariable(FloatVariableName, Dialogue, EDlgTextArgumentType::ClassFloat, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified BoolVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForClassBoolVariableName(FName BoolVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(
			BoolVariableName,
			Dialogue,
			EDlgEventType::ModifyClassBoolVariable,
			EDlgConditionType::ClassBoolVariable
		);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForClassFNameVariableName(FName FNameVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(
			FNameVariableName,
			Dialogue,
			EDlgEventType::ModifyClassNameVariable,
			EDlgConditionType::ClassNameVariable
		);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForClassFTextVariableName(FName FTextVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDlgSearchFoundResult> FoundResult = FDlgSearchFoundResult::Make();
		GetGraphNodesForTextArgumentVariable(FTextVariableName, Dialogue, EDlgTextArgumentType::ClassText, FoundResult);
		return FoundResult;
	}

	// Does Conditions contain the ConditionName (of type ConditionType)?
	static bool IsConditionInArray(
		FName ConditionName,
		EDlgConditionType ConditionType,
		const TArray<FDlgCondition>& Conditions
	)
	{
		for (const FDlgCondition& Condition : Conditions)
		{
			// Matches the First participant
			if (Condition.CallbackName == ConditionName && Condition.ConditionType == ConditionType)
			{
				return true;
			}

			// Matches the second Participant
			if (Condition.CompareType != EDlgCompare::ToConst && Condition.OtherVariableName == ConditionName)
			{
				if (FDlgCondition::IsSameValueType(ConditionType, Condition.ConditionType))
				{
					return true;
				}
			}
		}
		return false;
	}

	// Does Events contain the EventName (of type EventType)?
	static bool IsEventInArray(FName EventName, EDlgEventType EventType, const TArray<FDlgEvent>& Events)
	{
		for (const FDlgEvent& Event : Events)
		{
			if (Event.EventType == EventType && Event.EventName == EventName)
			{
				return true;
			}
		}
		return false;
	}

	// Does the Events contain a Custom Event of Class EventClass
	static bool IsCustomEventInArray(const UClass* EventClass,  const TArray<FDlgEvent>& Events)
	{
		for (const FDlgEvent& Event : Events)
		{
			if (Event.EventType == EDlgEventType::Custom && Event.CustomEvent && Event.CustomEvent->GetClass() == EventClass)
			{
				return true;
			}
		}
		return false;
	}

	// Does Events contain the EventName (of type EventType)?
	static bool IsTextArgumentInArray(FName TextArgumentName, EDlgTextArgumentType TextArgumentType, const TArray<FDlgTextArgument>& TextArguments)
	{
		for (const FDlgTextArgument& TextArgument : TextArguments)
		{
			if (TextArgument.Type == TextArgumentType && TextArgument.VariableName == TextArgumentName)
			{
				return true;
			}
		}
		return false;
	}

	// Does the SearchString exist in the GUID? We test every possible format
	// In case of success we return the GUID as a string in OutStringGUID
	static bool DoesGUIDContainString(const FGuid& GUID, const FString& SearchString, FString& OutGUIDString);

	// Does the SearchString exist in the GUID Class name?
	// In case of success we return the GUID as a string in OutNameString
	static bool DoesObjectClassNameContainString(const UObject* Object, const FString& SearchString, FString& OutNameString);

private:
	static TSharedPtr<FDlgSearchFoundResult> GetGraphNodesForVariablesOfNameAndType(
		FName VariableName,
		const UDlgDialogue* Dialogue,
		EDlgEventType EventType,
		EDlgConditionType ConditionType
	);

	static void GetGraphNodesForTextArgumentVariable(
		FName VariableName,
		const UDlgDialogue* Dialogue,
		EDlgTextArgumentType ArgumentType,
		TSharedPtr<FDlgSearchFoundResult>& FoundResult
	);
};
