// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreTypes.h"

#include "DlgCondition.h"
#include "DlgEvent.h"
#include "DlgTextArgument.h"

#include "DlgEdge.generated.h"


class UDlgContextInternal;
class UDlgNode;

/**
 * The representation of a child in a node. Defined by a TargetIndex which points to the index array in the Dialogue.Nodes
 */
USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgEdge
{
	GENERATED_USTRUCT_BODY()

public:
	bool operator==(const FDlgEdge& Other) const
	{
		return TargetIndex == Other.TargetIndex &&
			Text.EqualTo(Other.Text) &&
			Conditions == Other.Conditions;
	}

	bool operator!=(const FDlgEdge& Other) const
	{
		return !(*this == Other);
	}

	// Operator overload for serialization
	friend FArchive& operator<<(FArchive &Ar, FDlgEdge& DlgEdge);

	/** Creates a simple edge without text, without conditions */
	FDlgEdge(int32 InTargetIndex = INDEX_NONE) : TargetIndex(InTargetIndex) {}

	// Rebuilds TextArguments
	void RebuildTextArgumentsArray() { FDlgTextArgument::UpdateTextArgumentArray(Text, TextArguments); }

	/** Returns with true if every condition attached to the edge and every enter condition of the target node are satisfied */
	bool Evaluate(const UDlgContextInternal* DlgContext, TSet<const UDlgNode*> AlreadyVisitedNodes) const;

	/** Constructs the ConstructedText. */
	void ConstructTextFromArguments(const UDlgContextInternal* DlgContext, FName NodeOwnerName);

	/** Gets the edge text. Empty text, or Text formatted with the text arguments if there is any, when the parent node is entered. */
	const FText& GetEdgeText() const { return (TextArguments.Num() > 0 && !ConstructedText.IsEmpty()) ? ConstructedText : Text; }

	/** Returns if the Edge is valid, has the TargetIndex non negative  */
	bool IsValid() const
	{
		return TargetIndex > INDEX_NONE;
	}

	static const FDlgEdge& GetInvalidEdge();

public:
	/** Index of the node in the Nodes TArray of the dialogue this edge is leading to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DialogueEdgeData, Meta = (ClampMin = -1))
	int32 TargetIndex = INDEX_NONE;

	/** Required but not sufficient conditions - target node's enter conditions are checked too */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueEdgeData)
	TArray<FDlgCondition> Conditions;

	/** Text associated with the child, can be used for user choices */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueEdgeData, Meta = (MultiLine = true))
	FText Text;

	UPROPERTY(EditAnywhere, EditFixedSize, Category = DialogueEdgeData)
	TArray<FDlgTextArgument> TextArguments;

	/** player emotion/state attached to this player choice */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueEdgeData)
	FName SpeakerState;

	/** Set this to false in order to skip this edge in the AllChildren array (which lists both satisfied and not satisfied player choices */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueEdgeData)
	bool bIncludeInAllOptionListIfUnsatisfied = true;

	/** Constructed at runtime from the original text and the arguments if there is any.*/
	FText ConstructedText;
};
