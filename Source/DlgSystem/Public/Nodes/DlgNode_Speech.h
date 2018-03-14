// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgNode.h"
#include "DlgNode_Speech.generated.h"

class USoundWave;
class UDialogueWave;

/**
 * Normal dialogue node - someone says something.
 */
UCLASS(BlueprintType)
class DLGSYSTEM_API UDlgNode_Speech : public UDlgNode
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.
	FString GetDesc() override
	{
		if (bIsVirtualParent)
			return TEXT("Virtual Parent Node. Acts like a fake parent (proxy) to other child nodes. (aka makes it get the grandchildren)\nOn revaluate children, it does not get the direct children but the children of the first satisfied direct child node (grandchildren).\nIt should have at least one satisified child otherwise the Dialogue is terminated.");

		return TEXT("Normal dialogue node - someone says something.");
	}

	// Begin UDlgNode Interface.
	bool ReevaluateChildren(UDlgContextInternal* DlgContext, TSet<UDlgNode*> AlreadyEvaluated) override;

	// Getters:
	const FText& GetNodeText() const override { return Text; }
	USoundWave* GetNodeVoiceSoundWave() const override { return VoiceSoundWave; }
	UDialogueWave* GetNodeVoiceDialogueWave() const override { return VoiceDialogueWave; }
	FName GetSpeakerState() const override { return SpeakerState; }
	void AddSpeakerStates(TSet<FName>& States) const { States.Add(SpeakerState); }

#if WITH_EDITOR
	FString GetNodeTypeString() const override { return bIsVirtualParent ? TEXT("Virtual Parent") : TEXT("Speech"); }
#endif

	// Begin own functions.
	/** Is this node a virtual parent? */
	virtual bool IsVirtualParent() const { return bIsVirtualParent; }

	/** Sets the virtual parent status */
	virtual void SetIsVirtualParent(bool bValue) { bIsVirtualParent = bValue; }

	/** Sets the Text of the Node. */
	virtual void SetNodeText(const FText& InText) { Text = InText; }

	/** Helper functions to get the names of some properties. Used by the DlgSystemEditor module. */
	static FName GetMemberNameText() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, Text); }
	static FName GetMemberNameVoiceSoundWave() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, VoiceSoundWave); }
	static FName GetMemberNameVoiceDialogueWave() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, VoiceDialogueWave); }
	static FName GetMemberNameSpeakerState() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, SpeakerState); }
	static FName GetMemberNameIsVirtualParent() { return GET_MEMBER_NAME_CHECKED(UDlgNode_Speech, bIsVirtualParent); }

protected:

	/** Text that will appear when this node participant name speaks to someone else. */
	UPROPERTY(EditAnywhere, Category = DlgNodeData, Meta = (MultiLine = true))
	FText Text;

	/** Voice attached to this node. The Sound Wave variant. */
	UPROPERTY(EditAnywhere, Category = DlgNodeData, Meta = (DlgSaveOnlyReference))
	USoundWave* VoiceSoundWave;

	/** Voice attached to this node. The Dialogue Wave variant. Only the first wave from the dialogue context array should be used. */
	UPROPERTY(EditAnywhere, Category = DlgNodeData, Meta = (DlgSaveOnlyReference))
	UDialogueWave* VoiceDialogueWave;

	/** State of the speaker attached to this node. Passed to the GetParticipantIcon function. */
	UPROPERTY(EditAnywhere, Category = DlgNodeData)
	FName SpeakerState;

	/**
	 * Make this Node act like a fake parent (proxy) to other child nodes. (aka makes it get the grandchildren)
	 * On revaluate children, it does not get the direct children but the children of the first satisfied direct child node (grandchildren).
	 * It should have at least one satisfied child otherwise the Dialogue is terminated.
	 */
	UPROPERTY(EditAnywhere, Category = DlgNodeData)
	bool bIsVirtualParent = false;
};
