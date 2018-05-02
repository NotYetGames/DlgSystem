// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DlgManager.h"

#define CREATE_VISIBILITY_CALLBACK(_SelfMethod) \
	TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, _SelfMethod))

// Constants used in this file
static constexpr const TCHAR* META_ShowOnlyInnerProperties = TEXT("ShowOnlyInnerProperties");
static constexpr const TCHAR* META_UIMin = TEXT("UIMin");
static constexpr const TCHAR* META_UIMax = TEXT("UIMax");
static constexpr const TCHAR* META_ClampMin = TEXT("ClampMin");
static constexpr const TCHAR* META_ClampMax = TEXT("ClampMax");

struct FDialogueDetailsPanelUtils
{
public:
	/** Gets the appropriate modifier key for an input field depending on the Dialogue System Settings */
	static EModifierKey::Type GetModifierKeyFromDialogueSettings()
	{
		switch (GetDefault<UDlgSystemSettings>()->DialogueTextInputKeyForNewLine)
		{
		case EDlgTextInputKeyForNewLine::DlgTextInputKeyForNewLineShiftPlusEnter:
			return EModifierKey::Shift;

		case EDlgTextInputKeyForNewLine::DlgTextInputKeyForNewLineEnter:
		default:
			return EModifierKey::None;
		}
	}

	/** Resets the numeric property to not have any limits */
	static void ResetNumericPropertyLimits(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		if (!PropertyHandle.IsValid())
		{
			return;
		}

		UProperty* Property = PropertyHandle->GetProperty();
		Property->RemoveMetaData(META_UIMin);
		Property->RemoveMetaData(META_UIMax);
		Property->RemoveMetaData(META_ClampMin);
		Property->RemoveMetaData(META_ClampMax);
	}

	/** Sets the limits of the numeric property. It can only have values in the range [Min, Max] */
	template <typename NumericType>
	static void SetNumericPropertyLimits(TSharedPtr<IPropertyHandle> PropertyHandle, const NumericType Min, const NumericType Max)
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
		UProperty* Property = PropertyHandle->GetProperty();

		// min
		Property->SetMetaData(META_UIMin, *MinString);
		Property->SetMetaData(META_ClampMin, *MinString);

		// max
		Property->SetMetaData(META_UIMax, *MaxString);
		Property->SetMetaData(META_ClampMax, *MaxString);
	}

	/** Gets the Base GraphNode owner that belongs to this PropertyHandle. It could be an Edge or a GraphNode */
	static UDialogueGraphNode_Base* GetGraphNodeBaseFromPropertyHandle(const TSharedRef<IPropertyHandle> PropertyHandle)
	{
		TArray<UObject*> OuterObjects;
		PropertyHandle->GetOuterObjects(OuterObjects);

		for (UObject* Object : OuterObjects)
		{
			if (UDlgNode* Node = Cast<UDlgNode>(Object))
			{
				return CastChecked<UDialogueGraphNode_Base>(Node->GetGraphNode());
			}

			if (UDialogueGraphNode_Base* Node = Cast<UDialogueGraphNode_Base>(Object))
			{
				return Node;
			}
		}

		return nullptr;
	}

	/**
	 * Similar to the Base node only this always returns a UDialogueGraphNode
	 * If the BaseGraphNode is an GraphNode then return that
	 * If the BaseGraphNode is an Edge then return the ParentGraphNode
	 */
	static UDialogueGraphNode* GetClosestGraphNodeFromPropertyHandle(const TSharedRef<IPropertyHandle> PropertyHandle)
	{
		if (UDialogueGraphNode_Base* BaseGraphNode = GetGraphNodeBaseFromPropertyHandle(PropertyHandle))
		{
			if (UDialogueGraphNode* Node = Cast<UDialogueGraphNode>(BaseGraphNode))
			{
				return Node;
			}
			if (UDialogueGraphNode_Edge* GraphEdge = Cast<UDialogueGraphNode_Edge>(BaseGraphNode))
			{
				if (GraphEdge->HasParentNode())
				{
					return GraphEdge->GetParentNode();
				}
			}
		}

		return nullptr;
	}

	/**
	 * Similar to the Base node only this always returns a UDialogueGraphNode_Edge
	 * If the BaseGraphNode is an GraphNode then returns nullptr
	 * If the BaseGraphNode is an Edge then returns that
	 */
	static UDialogueGraphNode_Edge* GetAsGraphNodeEdgeFromPropertyHandle(const TSharedRef<IPropertyHandle> PropertyHandle)
	{
		return Cast<UDialogueGraphNode_Edge>(GetGraphNodeBaseFromPropertyHandle(PropertyHandle));
	}

	/** Gets the Dialogue that is the top most root owner of this PropertyHandle. used in the details panel. */
	static UDlgDialogue* GetDialogueFromPropertyHandle(const TSharedRef<IPropertyHandle> PropertyHandle)
	{
		UDlgDialogue* Dialogue = nullptr;

		// Check first children objects of property handle, should be a dialogue node or a graph node
		if (UDialogueGraphNode_Base* GraphNode = GetGraphNodeBaseFromPropertyHandle(PropertyHandle))
		{
			Dialogue = GraphNode->GetDialogue();
		}

		// One last try, get to the root of the problem ;)
		if (!IsValid(Dialogue))
		{
			TSharedPtr<IPropertyHandle> ParentHandle = PropertyHandle->GetParentHandle();
			// Find the root property handle
			while (ParentHandle.IsValid() && ParentHandle->GetParentHandle().IsValid())
			{
				ParentHandle = ParentHandle->GetParentHandle();
			}

			// The outer should be a dialogue
			if (ParentHandle.IsValid())
			{
				TArray<UObject*> OuterObjects;
				ParentHandle->GetOuterObjects(OuterObjects);
				for (UObject* Object : OuterObjects)
				{
					if (UDlgDialogue* FoundDialogue = Cast<UDlgDialogue>(Object))
					{
						Dialogue = FoundDialogue;
						break;
					}
				}
			}
		}

		check(Dialogue);
		return Dialogue;
	}

	/**
	 * Tries to get the participant name of the struct by using that structs PropertyName
	 * 1. Tries to get the value from the ParticipantNamePropertyHandle of that struct.
	 * 2. Gets the ParticipantName from the Node that has this property.
	 */
	static FName GetParticipantNameFromPropertyHandle(TSharedRef<IPropertyHandle> ParticipantNamePropertyHandle)
	{
		FName ParticipantName = NAME_None;
		if (ParticipantNamePropertyHandle->GetValue(ParticipantName) != FPropertyAccess::Success)
		{
			return ParticipantName;
		}

		// Try the node that owns this
		if (ParticipantName.IsNone())
		{
			// Possible edge?
			if (UDialogueGraphNode* GraphNode = GetClosestGraphNodeFromPropertyHandle(ParticipantNamePropertyHandle))
			{
				return GraphNode->GetDialogueNode().GetNodeParticipantName();
			}
		}

		return ParticipantName;
	}

	/** Gets all the participant names of the Dialogue sorted alphabetically */
	static TArray<FName> GetDialogueSortedParticipantNames(UDlgDialogue* Dialogue)
	{
		TSet<FName> ParticipantNames;
		Dialogue->GetAllParticipantNames(ParticipantNames);
		FDlgHelper::SortDefault(ParticipantNames);
		return ParticipantNames.Array();
	}
};
