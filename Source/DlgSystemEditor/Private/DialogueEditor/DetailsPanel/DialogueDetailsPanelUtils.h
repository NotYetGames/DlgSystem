// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"

#define CREATE_VISIBILITY_CALLBACK(_SelfMethod) \
	TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, _SelfMethod))

#define CREATE_VISIBILITY_CALLBACK_STATIC(_StaticMethod) \
	TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateStatic(_StaticMethod))

#define CREATE_BOOL_CALLBACK(_SelfMethod) \
	TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, _SelfMethod))

#define CREATE_BOOL_CALLBACK_STATIC(_StaticMethod) \
	TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateStatic(_StaticMethod))

// Constants used in this file
static const TCHAR* META_ShowOnlyInnerProperties = TEXT("ShowOnlyInnerProperties");
static const TCHAR* META_UIMin = TEXT("UIMin");
static const TCHAR* META_UIMax = TEXT("UIMax");
static const TCHAR* META_ClampMin = TEXT("ClampMin");
static const TCHAR* META_ClampMax = TEXT("ClampMax");

struct FDialogueDetailsPanelUtils
{
public:
	// Getters for visibility of some properties
	static EVisibility GetVoiceSoundWaveVisibility()
	{
		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		return Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::SoundWave ||
			   Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::SoundWaveAndDialogueWave
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	static EVisibility GetVoiceDialogueWaveVisibility()
	{
		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		return Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::DialogueWave ||
			   Settings->DialogueDisplayedVoiceFields == EDlgVoiceDisplayedFields::SoundWaveAndDialogueWave
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	static EVisibility GetSpeakerStateNodeVisibility()
	{
		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		return Settings->DialogueSpeakerStateVisibility == EDlgSpeakerStateVisibility::ShowOnNode ||
			   Settings->DialogueSpeakerStateVisibility == EDlgSpeakerStateVisibility::ShowOnNodeAndEdge
			   ? EVisibility::Visible : EVisibility::Hidden;
	}

	static EVisibility GetNodeDataVisibility()
	{
		return GetDefault<UDlgSystemSettings>()->bShowNodeData ? EVisibility::Visible : EVisibility::Hidden;
	}

	static EVisibility GetNodeGenericDataVisibility()
	{
		return GetDefault<UDlgSystemSettings>()->bShowGenericData ? EVisibility::Visible : EVisibility::Hidden;
	}

	static EVisibility GetChildrenVisibility()
	{
		return GetDefault<UDlgSystemSettings>()->bShowAdvancedChildren ? EVisibility::Visible : EVisibility::Hidden;
	}

	/** Gets the appropriate modifier key for an input field depending on the Dialogue System Settings */
	static EModifierKey::Type GetModifierKeyFromDialogueSettings()
	{
		switch (GetDefault<UDlgSystemSettings>()->DialogueTextInputKeyForNewLine)
		{
		case EDlgTextInputKeyForNewLine::ShiftPlusEnter:
			return EModifierKey::Shift;

		case EDlgTextInputKeyForNewLine::Enter:
		default:
			return EModifierKey::None;
		}
	}

	/** Resets the numeric property to not have any limits */
	static void ResetNumericPropertyLimits(const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		if (!PropertyHandle.IsValid())
		{
			return;
		}

		auto* Property = PropertyHandle->GetProperty();
		Property->RemoveMetaData(META_UIMin);
		Property->RemoveMetaData(META_UIMax);
		Property->RemoveMetaData(META_ClampMin);
		Property->RemoveMetaData(META_ClampMax);
	}

	/** Sets the limits of the numeric property. It can only have values in the range [Min, Max] */
	template <typename NumericType>
	static void SetNumericPropertyLimits(const TSharedPtr<IPropertyHandle>& PropertyHandle, const NumericType Min, const NumericType Max)
	{
		if (!PropertyHandle.IsValid())
		{
			return;
		}

		// Clamp Current value if not in range
		NumericType NumericValue;
		if (PropertyHandle->GetValue(NumericValue) != FPropertyAccess::Success)
		{
			return;
		}
		if (PropertyHandle->SetValue(FMath::Clamp(NumericValue, Min, Max)) != FPropertyAccess::Success)
		{
			return;
		}

		const FString MinString = FString::FromInt(Min);
		const FString MaxString = FString::FromInt(Max);
		auto* Property = PropertyHandle->GetProperty();

		// min
		Property->SetMetaData(META_UIMin, *MinString);
		Property->SetMetaData(META_ClampMin, *MinString);

		// max
		Property->SetMetaData(META_UIMax, *MaxString);
		Property->SetMetaData(META_ClampMax, *MaxString);
	}

	/** Gets the Base GraphNode owner that belongs to this PropertyHandle. It could be an Edge or a GraphNode */
	static UDialogueGraphNode_Base* GetGraphNodeBaseFromPropertyHandle(const TSharedRef<IPropertyHandle>& PropertyHandle);

	/**
	 * Similar to the Base node only this always returns a UDialogueGraphNode
	 * If the BaseGraphNode is an GraphNode then return that
	 * If the BaseGraphNode is an Edge then return the ParentGraphNode
	 */
	static UDialogueGraphNode* GetClosestGraphNodeFromPropertyHandle(const TSharedRef<IPropertyHandle>& PropertyHandle);

	/**
	 * Similar to the Base node only this always returns a UDialogueGraphNode_Edge
	 * If the BaseGraphNode is an GraphNode then returns nullptr
	 * If the BaseGraphNode is an Edge then returns that
	 */
	static UDialogueGraphNode_Edge* GetAsGraphNodeEdgeFromPropertyHandle(const TSharedRef<IPropertyHandle>& PropertyHandle)
	{
		return Cast<UDialogueGraphNode_Edge>(GetGraphNodeBaseFromPropertyHandle(PropertyHandle));
	}

	/** Gets the Dialogue that is the top most root owner of this PropertyHandle. used in the details panel. */
	static UDlgDialogue* GetDialogueFromPropertyHandle(const TSharedRef<IPropertyHandle>& PropertyHandle);

	/**
	 * Tries to get the participant name of the struct by using that structs PropertyName
	 * 1. Tries to get the value from the ParticipantNamePropertyHandle of that struct.
	 * 2. Gets the ParticipantName from the Node that has this property.
	 */
	static FName GetParticipantNameFromPropertyHandle(const TSharedRef<IPropertyHandle>& ParticipantNamePropertyHandle);

	/** Gets all the participant names of the Dialogue sorted alphabetically */
	static TArray<FName> GetDialogueSortedParticipantNames(UDlgDialogue* Dialogue);
};
