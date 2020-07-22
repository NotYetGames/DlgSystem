// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Nodes/DlgNode.h"
#include "DlgTextArgument.h"
#include "DlgNode_Speech.generated.h"

class USoundWave;
class UDialogueWave;
struct FDlgTextArgument;

/**
 * Normal dialogue node - someone says something.
 */
UCLASS(BlueprintType, ClassGroup = "Dialogue")
class DLGSYSTEM_API UDlgNode_Speech : public UDlgNode
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.
	FString GetDesc() override
	{
		if (bIsVirtualParent)
		{
			return TEXT("Virtual Parent Node. Acts like a fake parent (proxy) to other child nodes. (aka makes it get the grandchildren)\nOn revaluate children, it does not get the direct children but the children of the first satisfied direct child node (grandchildren).\nIt should have at least one satisified child otherwise the Dialogue is terminated.");
		}

		return TEXT("Normal dialogue node - someone says something.");
	}

#if WITH_EDITOR
	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	//
	// Begin UDlgNode Interface.
	//

	bool HandleNodeEnter(UDlgContext& Context, TSet<const UDlgNode*> NodesEnteredWithThisStep) override;
	bool ReevaluateChildren(UDlgContext& Context, TSet<const UDlgNode*> AlreadyEvaluated) override;
	void GetAssociatedParticipants(TArray<FName>& OutArray) const override;

	void UpdateTextsValuesFromDefaultsAndRemappings(const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode = true) override;
	void UpdateTextsNamespacesAndKeys(const UDlgSystemSettings& Settings, bool bEdges, bool bUpdateGraphNode = true) override;
	void RebuildConstructedText(const UDlgContext& Context) override;
	void RebuildTextArguments(bool bEdges, bool bUpdateGraphNode = true) override
	{
		Super::RebuildTextArguments(bEdges, bUpdateGraphNode);
		FDlgTextArgument::UpdateTextArgumentArray(Text, TextArguments);
	}
	void RebuildTextArgumentsFromPreview(const FText& Preview) override { FDlgTextArgument::UpdateTextArgumentArray(Preview, TextArguments); }
	const TArray<FDlgTextArgument>& GetTextArguments() const override { return TextArguments; };

	// Getters:
	const FText& GetNodeText() const override
	{
		if (TextArguments.Num() > 0 && !ConstructedText.IsEmpty())
		{
			return ConstructedText;
		}
		return Text;
	}
	const FText& GetNodeUnformattedText() const override { return Text; }
	UDlgNodeData* GetNodeData() const override { return NodeData; }

	// stuff we have to keep for legacy reason (but would make more sense to remove them from the plugin as they could be created in NodeData):
	FName GetSpeakerState() const override { return SpeakerState; }
	USoundBase* GetNodeVoiceSoundBase() const override { return VoiceSoundWave; }
	UDialogueWave* GetNodeVoiceDialogueWave() const override { return VoiceDialogueWave; }
	UObject* GetNodeGenericData() const override { return GenericData; }

	void AddAllSpeakerStatesIntoSet(TSet<FName>& OutStates) const override { OutStates.Add(SpeakerState); }

#if WITH_EDITOR
	FString GetNodeTypeString() const override { return bIsVirtualParent ? TEXT("Virtual Parent") : TEXT("Speech"); }
#endif

	//
	// Begin own functions.
	//

	// Is this node a virtual parent?
	UFUNCTION(BlueprintPure, Category = "Dialogue|Node")
	virtual bool IsVirtualParent() const { return bIsVirtualParent; }

	// Sets the virtual parent status
	virtual void SetIsVirtualParent(bool bValue) { bIsVirtualParent = bValue; }

	// Sets the RawNodeText of the Node and rebuilds the constructed text
	virtual void SetNodeText(const FText& InText)
	{
		Text = InText;
		RebuildTextArguments(false);
	}

	void SetNodeData(UDlgNodeData* InNodeData) { NodeData = InNodeData; }
	void SetSpeakerState(FName InSpeakerState) { SpeakerState = InSpeakerState; }
	void SetVoiceSoundBase(USoundBase* InVoiceSoundBase) { VoiceSoundWave = InVoiceSoundBase; }
	void SetVoiceDialogueWave(UDialogueWave* InVoiceDialogueWave) { VoiceDialogueWave = InVoiceDialogueWave; }
	void SetGenericData(UObject* InGenericData) { GenericData = InGenericData; }

	// Helper functions to get the names of some properties. Used by the DlgSystemEditor module.
	static FName GetMemberNameText() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, Text); }
	static FName GetMemberNameTextArguments() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, TextArguments); }
	static FName GetMemberNameNodeData() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, NodeData); }
	static FName GetMemberNameVoiceSoundWave() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, VoiceSoundWave); }
	static FName GetMemberNameVoiceDialogueWave() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, VoiceDialogueWave); }
	static FName GetMemberNameGenericData() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, GenericData); }
	static FName GetMemberNameSpeakerState() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, SpeakerState); }
	static FName GetMemberNameIsVirtualParent() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, bIsVirtualParent); }
	static FName GetMemberNameVirtualParentFireDirectChildEnterEvents() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, bVirtualParentFireDirectChildEnterEvents); }

protected:
	// Make this Node act like a fake parent (proxy) to other child nodes (makes it get the granchildren).
	// On reevaluate children, it does not get the direct children but the children of the first satisfied direct child node (grandchildren).
	//
	// NOTE: It should have at least one satisfied child otherwise the Dialogue is terminated.
	// NOTE: The first satisfied direct child will be added to the visited history when this node is entered.
	// NOTE: Most Common usage for this is to make loops.
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node")
	bool bIsVirtualParent = false;

	// If true the first satisfied Direct Child of this Virtual Parent Node will fire its Enter Events after this node is entered (and fires its enter events).
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node")
	bool bVirtualParentFireDirectChildEnterEvents = true;

	// Text that will appear when this node participant name speaks to someone else.
	// If you want replaceable portions inside your Text nodes just add {identifier} inside it and set the value it should have at runtime.
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node", Meta = (MultiLine = true))
	FText Text;

	// If you want replaceable portions inside your Text nodes just add {identifier} inside it and set the value it should have at runtime.
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "Dialogue|Node")
	TArray<FDlgTextArgument> TextArguments;

	// State of the speaker attached to this node. Passed to the GetParticipantIcon function.
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node")
	FName SpeakerState;

	// User Defined Custom Node data you can customize yourself with your own data types
	//
	// Create a new Blueprint derived from DlgNodeData (or DlgNodeDataHideCategories)
	UPROPERTY(EditAnywhere, Instanced, Category = "Dialogue|Node")
	UDlgNodeData* NodeData = nullptr;

	// Voice attached to this node. The Sound Wave variant.
	// NOTE: You should probably use the NodeData
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node", Meta = (DlgSaveOnlyReference))
	USoundBase* VoiceSoundWave = nullptr;

	// Voice attached to this node. The Dialogue Wave variant. Only the first wave from the dialogue context array should be used.
	// NOTE: You should probably use the NodeData
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node", Meta = (DlgSaveOnlyReference))
	UDialogueWave* VoiceDialogueWave = nullptr;

	// Any generic object you would like
	// NOTE: You should probably use the NodeData
	UPROPERTY(EditAnywhere, Category = "Dialogue|Node", Meta = (DlgSaveOnlyReference))
	UObject* GenericData = nullptr;

	// Constructed at runtime from the original text and the arguments if there is any.
	FText ConstructedText;

	int32 VirtualParentFirstSatisfiedDirectChildIndex = INDEX_NONE;
};
