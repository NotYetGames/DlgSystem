// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DlgEvent.h"
#include "DlgCondition.h"
#include "DlgTextArgument.h"

//////////////////////////////////////////////////////////////////////////
// FDialogueESearchUtilities

class UDlgDialogue;
class UDialogueGraphNode;
struct FDlgNode;
class UEdGraph;
class UDialogueGraphNode_Base;
class UDialogueGraphNode_Edge;

// Represents the found result of the search functions.
struct FDialogueSearchFoundResult
{
private:
	typedef FDialogueSearchFoundResult Self;

public:
	static TSharedPtr<FDialogueSearchFoundResult> Make() { return MakeShared<Self>(); }

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
class FDialogueSearchUtilities
{
public:

	/**
	 * Gets all the graph nodes that contain the specified EventName (of the EventType DlgEventEvent)
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForEventEventName(const FName& EventName, const UDlgDialogue* Dialogue);

	/**
	 * Gets all the graph nodes that contain the specified ConditionName (of the ConditionType DlgConditionEventCall)
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForConditionEventCallName(const FName& ConditionName, const UDlgDialogue* Dialogue);

	/**
	 * Gets all the graph nodes that contain the specified IntVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForIntVariableName(const FName& IntVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDialogueSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(IntVariableName, Dialogue,
			EDlgEventType::DlgEventModifyInt, EDlgConditionType::DlgConditionIntCall);
		GetGraphNodesForTextArgumentVariable(IntVariableName, Dialogue, EDlgTextArgumentType::DlgTextArgumentDialogueInt, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified FloatVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForFloatVariableName(const FName& FloatVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDialogueSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(FloatVariableName, Dialogue,
			EDlgEventType::DlgEventModifyFloat, EDlgConditionType::DlgConditionFloatCall);
		GetGraphNodesForTextArgumentVariable(FloatVariableName, Dialogue, EDlgTextArgumentType::DlgTextArgumentDialogueInt, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified BoolVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForBoolVariableName(const FName& BoolVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(BoolVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyBool,
													  EDlgConditionType::DlgConditionBoolCall);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName basic type.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForFNameVariableName(const FName& FNameVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(FNameVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyName,
													  EDlgConditionType::DlgConditionNameCall);
	}

	/**
	 * Gets all the graph nodes that contain the specified IntVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForClassIntVariableName(const FName& IntVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDialogueSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(IntVariableName, Dialogue,
			EDlgEventType::DlgEventModifyClassIntVariable, EDlgConditionType::DlgConditionClassIntVariable);
		GetGraphNodesForTextArgumentVariable(IntVariableName, Dialogue, EDlgTextArgumentType::DlgTextArgumentClassInt, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified FloatVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForClassFloatVariableName(const FName& FloatVariableName, const UDlgDialogue* Dialogue)
	{

		TSharedPtr<FDialogueSearchFoundResult> FoundResult = GetGraphNodesForVariablesOfNameAndType(FloatVariableName, Dialogue,
			EDlgEventType::DlgEventModifyClassFloatVariable, EDlgConditionType::DlgConditionClassFloatVariable);
		GetGraphNodesForTextArgumentVariable(FloatVariableName, Dialogue, EDlgTextArgumentType::DlgTextArgumentClassFloat, FoundResult);
		return FoundResult;
	}

	/**
	 * Gets all the graph nodes that contain the specified BoolVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForClassBoolVariableName(const FName& BoolVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(BoolVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyClassBoolVariable,
													  EDlgConditionType::DlgConditionClassBoolVariable);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForClassFNameVariableName(const FName& FNameVariableName, const UDlgDialogue* Dialogue)
	{
		return GetGraphNodesForVariablesOfNameAndType(FNameVariableName, Dialogue,
													  EDlgEventType::DlgEventModifyClassNameVariable,
													  EDlgConditionType::DlgConditionClassNameVariable);
	}

	/**
	 * Gets all the graph nodes that contain the specified FNameVariableName from the UClass.
	 * This contains both graph nodes and edges.
	 */
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForClassFTextVariableName(const FName& FTextVariableName, const UDlgDialogue* Dialogue)
	{
		TSharedPtr<FDialogueSearchFoundResult> FoundResult = FDialogueSearchFoundResult::Make();
		GetGraphNodesForTextArgumentVariable(FTextVariableName, Dialogue, EDlgTextArgumentType::DlgTextArgumentClassText, FoundResult);
		return FoundResult;
	}

	/** Does Conditions contain the ConditionName (of type ConditionType)? */
	static bool IsConditionInArray(const FName& ConditionName, const EDlgConditionType ConditionType,
								   const TArray<FDlgCondition>& Conditions)
	{
		for (const FDlgCondition& Condition : Conditions)
		{
			if (Condition.ConditionType == ConditionType &&
				(Condition.CallbackName == ConditionName || Condition.OtherVariableName == ConditionName))
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

	/** Does Events contain the EventName (of type EventType)? */
	static bool IsTextArgumentInArray(const FName& TextArgumentName, const EDlgTextArgumentType TextArgumentType, const TArray<FDlgTextArgument>& TextArguments)
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

private:
	static TSharedPtr<FDialogueSearchFoundResult> GetGraphNodesForVariablesOfNameAndType(const FName& VariableName,
																				const UDlgDialogue* Dialogue,
																				const EDlgEventType EventType,
																				const EDlgConditionType ConditionType);

	static void GetGraphNodesForTextArgumentVariable(const FName& VariableName,
													 const UDlgDialogue* Dialogue,
													 const EDlgTextArgumentType ArgumentType,
													 TSharedPtr<FDialogueSearchFoundResult>& FoundResult);
};
