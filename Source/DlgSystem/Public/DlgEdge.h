// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreTypes.h"

#include "DlgCondition.h"
#include "DlgEvent.h"
#include "DlgTextArgument.h"

#include "DlgEdge.generated.h"

class UDlgSystemSettings;
class UDlgContext;
class UDlgNode;
class UDlgDialogue;
class UDlgNodeData;

/**
 * The representation of a child in a node. Defined by a TargetIndex which points to the index array in the Dialogue.Nodes
 */
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgEdge
{
	GENERATED_USTRUCT_BODY()

public:
	// Creates a simple edge without text, without conditions
	FDlgEdge(int32 InTargetIndex = INDEX_NONE) : TargetIndex(InTargetIndex) {}

	//
	// ICppStructOps Interface
	//

	bool operator==(const FDlgEdge& Other) const
	{
		return TargetIndex == Other.TargetIndex &&
			SpeakerState == Other.SpeakerState &&
			Text.EqualTo(Other.Text) &&
			bIncludeInAllOptionListIfUnsatisfied == Other.bIncludeInAllOptionListIfUnsatisfied &&
			TextArguments == Other.TextArguments &&
			Conditions == Other.Conditions;
	}

	bool operator!=(const FDlgEdge& Other) const
	{
		return !(*this == Other);
	}

	//
	// Own methods
	//

	// Is the Text property visible on this edge, the edges comes from the ParentNode
	static bool IsTextVisible(const UDlgNode& ParentNode);

	// Updates the text value of the Edge Text from the default value and text remapping (if any)
	void UpdateTextValueFromDefaultAndRemapping(
		const UDlgDialogue& ParentDialogue, const UDlgNode& ParentNode, const UDlgSystemSettings& Settings, bool bUpdateFromRemapping
	);

	// Updates the namespace or keys depending on the settings
	void UpdateTextsNamespacesAndKeys(const UObject* ParentObject, const UDlgSystemSettings& Settings);

	// Rebuilds TextArguments
	void RebuildTextArguments() { FDlgTextArgument::UpdateTextArgumentArray(Text, TextArguments); }
	void RebuildTextArgumentsFromPreview(const FText& Preview) { FDlgTextArgument::UpdateTextArgumentArray(Preview, TextArguments); }

	// Returns with true if every condition attached to the edge and every enter condition of the target node are satisfied //
	bool Evaluate(const UDlgContext& Context, TSet<const UDlgNode*> AlreadyVisitedNodes) const;

	// Constructs the ConstructedText.
	void RebuildConstructedText(const UDlgContext& Context, FName FallbackParticipantName);

	const TArray<FDlgTextArgument>& GetTextArguments() const { return TextArguments; }

	// Sets the text and rebuilds the formatted constructed text
	void SetText(const FText& NewText)
	{
		SetUnformattedText(NewText);
		RebuildTextArguments();
	}

	// Sets the unformatted text, this is the text that includes the {identifier}
	// NOTE: this is not call RebuildTextArguments(), use SetText for that
	void SetUnformattedText(const FText& NewText)
	{
		Text = NewText;
	}

	/** Gets the edge text. Empty text, or Text formatted with the text arguments if there is any, when the parent node is entered. */
	const FText& GetText() const
	{
		if (TextArguments.Num() > 0 && !ConstructedText.IsEmpty())
		{
			return ConstructedText;
		}
		return Text;
	}

	// This always returns the unformatted text, if you want the formatted text use GetText()
	const FText& GetUnformattedText() const { return Text; }
	FText& GetMutableUnformattedText() { return Text; }

	// Returns if the Edge is valid, has the TargetIndex non negative
	bool IsValid() const { return TargetIndex > INDEX_NONE; }

	static const FDlgEdge& GetInvalidEdge()
	{
		static FDlgEdge DlgEdge;
		return DlgEdge;
	}

	// Helper functions to get the names of some properties. Used by the DlgSystemEditor module.
	static FName GetMemberNameText() { return GET_MEMBER_NAME_CHECKED(FDlgEdge, Text); }
	static FName GetMemberNameTextArguments() { return GET_MEMBER_NAME_CHECKED(FDlgEdge, TextArguments); }
	static FName GetMemberNameEdgeData() { return GET_MEMBER_NAME_CHECKED(FDlgEdge, EdgeData); }

public:
	// Index of the node in the Nodes TArray of the dialogue this edge is leading to
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DialogueEdge", Meta = (ClampMin = -1))
	int32 TargetIndex = INDEX_NONE;

	// Required but not sufficient conditions - target node's enter conditions are checked too
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueEdge")
	TArray<FDlgCondition> Conditions;

	// Player emotion/state attached to this player choice
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueEdge")
	FName SpeakerState;

	// Set this to false in order to skip this edge in the AllChildren array (which lists both satisfied and not satisfied player choices
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueEdge")
	bool bIncludeInAllOptionListIfUnsatisfied = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "DialogueEdge")
	UDlgNodeData* EdgeData = nullptr;

protected:
	// Some Variables are here to stop misuse

	// Text associated with the child, can be used for user choices
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DialogueEdge", Meta = (MultiLine = true))
	FText Text;

	// If you want replaceable portions inside your Text nodes just add {identifier} inside it and set the value it should have at runtime.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, EditFixedSize, Category = "DialogueEdge")
	TArray<FDlgTextArgument> TextArguments;

	// Constructed at runtime from the original text and the arguments if there is any.
	FText ConstructedText;
};

template<>
struct TStructOpsTypeTraits<FDlgEdge> : public TStructOpsTypeTraitsBase2<FDlgEdge>
{
	enum
	{
		WithIdenticalViaEquality = true
	};
};
