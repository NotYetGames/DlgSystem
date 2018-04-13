// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DlgEvent.h"
#include "DlgCondition.h"

//////////////////////////////////////////////////////////////////////////
// FDialogueESearchUtilities

class UDlgDialogue;
class UDialogueGraphNode;
struct FDlgNode;
class UEdGraph;
class UDialogueGraphNode_Base;
class UDialogueGraphNode_Edge;
struct FDialogueSearchFoundResult;

typedef TSharedPtr<FDialogueSearchFoundResult> FDialogueSearchFoundResultPtr;

// Represents the found result of the search functions.
struct FDialogueSearchFoundResult
{
private:
	typedef FDialogueSearchFoundResult Self;

public:
	static FDialogueSearchFoundResultPtr Make() { return MakeShareable(new Self); }

public:
	// Nodes that satisfy the search result.
	TArray<TWeakObjectPtr<UDialogueGraphNode>> GraphNodes;

	// Edges that satisfy the search result.
	TArray<TWeakObjectPtr<UDialogueGraphNode_Edge>> EdgeNodes;
};

/**
 * Utilities for search.
 * @brief The FDialogueSearchUtilities class
 */
class FDialogueSearchUtilities
{
public:

	/**
	 * Gets all the graph nodes that contain the specified EventName (of the EventType DlgEventEvent)
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForEventEventName(const FName& EventName,
																		const UDlgDialogue* Dialogue);

	/**
	 * Gets all the graph nodes that contain the specified ConditionName (of the ConditionType DlgConditionEventCall)
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForConditionEventCallName(const FName& ConditionName,
																				const UDlgDialogue* Dialogue);

	/**
	 * Gets all the graph nodes that contain the specified IntVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForIntVariableName(const FName& IntVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(IntVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyInt,
													  EDlgConditionType::DlgConditionIntCall);
	}

	/**
	 * Gets all the graph nodes that contain the specified FloatVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForFloatVariableName(const FName& FloatVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(FloatVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyFloat,
													  EDlgConditionType::DlgConditionFloatCall);
	}

	/**
	 * Gets all the graph nodes that contain the specified BoolVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForBoolVariableName(const FName& BoolVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(BoolVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyBool,
													  EDlgConditionType::DlgConditionBoolCall);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForFNameVariableName(const FName& FNameVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(FNameVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyName,
													  EDlgConditionType::DlgConditionNameCall);
	}

	/**
	 * Gets all the graph nodes that contain the specified IntVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForClassIntVariableName(const FName& IntVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(IntVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyClassIntVariable,
													  EDlgConditionType::DlgConditionClassIntVariable);
	}

	/**
	 * Gets all the graph nodes that contain the specified FloatVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForClassFloatVariableName(const FName& FloatVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(FloatVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyClassFloatVariable,
													  EDlgConditionType::DlgConditionClassFloatVariable);
	}

	/**
	 * Gets all the graph nodes that contain the specified BoolVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForClassBoolVariableName(const FName& BoolVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(BoolVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyClassBoolVariable,
													  EDlgConditionType::DlgConditionClassBoolVariable);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static FDialogueSearchFoundResultPtr GetGraphNodesForClassFNameVariableName(const FName& FNameVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(FNameVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyClassNameVariable,
													  EDlgConditionType::DlgConditionClassNameVariable);
	}

	/** Does Conditions contain the ConditionName (of type ConditionType)? */
	static bool IsConditionInArray(const FName& ConditionName, const EDlgConditionType ConditionType,
								   const TArray<FDlgCondition>& Conditions)
	{
		for (const FDlgCondition& Condition : Conditions)
		{
			if (Condition.ConditionType == ConditionType && Condition.CallbackName == ConditionName)
			{
				return true;
			}
		}
		return false;
	}

	/** Does Events contain the EventName (of type EventType)? */
	static bool IsEventInArray(const FName& EventName, const EDlgEventType EventType, const TArray<FDlgEvent>& Events)
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

private:
	static FDialogueSearchFoundResultPtr GetGraphNodesForVariablesOfNameAndType(const FName& VariableName,
																				const UDlgDialogue* Dialogue,
																				const EDlgEventType EventType,
																				const EDlgConditionType ConditionType);
};
