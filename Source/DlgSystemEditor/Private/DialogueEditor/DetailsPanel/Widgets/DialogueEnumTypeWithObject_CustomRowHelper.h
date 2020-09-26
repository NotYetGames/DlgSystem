// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DialogueObject_CustomRowHelper.h"
#include "DlgCondition.h"
#include "DlgDialogueParticipant.h"
#include "DlgEvent.h"
#include "IDetailPropertyRow.h"

class FDetailWidgetRow;
class UDlgDialogue;
class UBlueprint;

enum class EDialogueEnumWithObjectType : uint8
{
	None = 0,
	Condition, // EDlgConditionType
	Event // EDlgEventType
};

// Widget:
// [Enum type] [Browse Participant] [Open Function or ParticipantBlueprint]
class FDialogueEnumTypeWithObject_CustomRowHelper : public FDialogueObject_CustomRowHelper
{
	typedef FDialogueEnumTypeWithObject_CustomRowHelper Self;
	typedef FDialogueObject_CustomRowHelper Super;

public:
	FDialogueEnumTypeWithObject_CustomRowHelper(
		IDetailPropertyRow* InPropertyRow,
		const UDlgDialogue* InDialogue,
		const TSharedPtr<IPropertyHandle>& InParticipantNamePropertyHandle
	) : Super(InPropertyRow), Dialogue(InDialogue), ParticipantNamePropertyHandle(InParticipantNamePropertyHandle)
	{
		bAddBlueprintFunctionIfItDoesNotExist = false;
	}
	virtual ~FDialogueEnumTypeWithObject_CustomRowHelper() {}

	//
	// FDialogueObject_CustomRowHelper Interface
	//

	UObject* GetObject() const override;
	FText GetBrowseObjectText() const override;
	FText GetJumpToObjectText() const override;
	FReply OnOpenClicked() override;
	bool CanBeVisible() const override;
	float GetRowMinimumDesiredWidth() const override { return 200.f; }

	//
	// Own methods
	//

	void SetEnumType(EDialogueEnumWithObjectType Type)
	{
		EnumType = Type;
	}

protected:
	bool CanBeVisibleForEventType() const
	{
		return IsEventTypeForADialogueValue() || IsEventTypeForAClassVariable();
	}

	bool CanBeVisibleForConditionType() const
	{
		return IsConditionTypeForADialogueValue() || IsConditionTypeForAClassVariable();
	}

	bool IsEventTypeForADialogueValue() const
	{
		const EDlgEventType EventType = GetEventType();
		return FDlgEvent::HasParticipantInterfaceValue(EventType);
	}

	bool IsEventTypeForAClassVariable() const
	{
		const EDlgEventType EventType = GetEventType();
		return FDlgEvent::HasClassVariable(EventType);
	}

	bool IsConditionTypeForADialogueValue() const
	{
		const EDlgConditionType ConditionType = GetConditionType();
		return FDlgCondition::HasParticipantInterfaceValue(ConditionType);
	}

	bool IsConditionTypeForAClassVariable() const
	{
		const EDlgConditionType ConditionType = GetConditionType();
		return FDlgCondition::HasClassVariable(ConditionType);
	}

	uint8 GetEnumValue() const;
	FName GetParticipantName() const;
	bool HasParticipantName() const { return GetParticipantName() != NAME_None; }
	EDlgEventType GetEventType() const { return static_cast<EDlgEventType>(GetEnumValue()); }
	EDlgConditionType GetConditionType() const {  return static_cast<EDlgConditionType>(GetEnumValue()); }


protected:
	EDialogueEnumWithObjectType EnumType = EDialogueEnumWithObjectType::None;
	TWeakObjectPtr<UDlgDialogue> Dialogue = nullptr;
	TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle = nullptr;
};
