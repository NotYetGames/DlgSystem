// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgDataPropertyValues.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SMissingWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"

#include "DlgSystem/NYReflectionHelper.h"
#include "UObject/TextProperty.h"

#define LOCTEXT_NAMESPACE "SDlgDataPropertyValues"

static FText ValidateNameLength(const FText& Text)
{
	if (Text.ToString().Len() > NAME_SIZE)
	{
		static FText ErrorString = FText::Format(LOCTEXT("NamePropertySizeTooLongError", "Name properties may only be a maximum of {0} characters"),
			FText::AsNumber(NAME_SIZE));
		return ErrorString;
	}

	return FText::GetEmpty();
}

static FString BoolToFString(const bool Value)
{
	return Value ? TEXT("True") : TEXT("False");
}

static bool FStringToBool(const FString& Value)
{
	return FCString::ToBool(*Value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataProperty
void SDlgDataPropertyValue::Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode)
{
	VariableNode = InVariableNode;
	if (!VariableNode.IsValid())
	{
		return;
	}

	UpdateVariableNodeFromActor();

	ChildSlot
	[
		SNew(STextBlock)
		.Text(this, &Self::GetTextValue)
	];
}

void SDlgDataPropertyValue::Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// We only run this Tick only after TickUpdateTimeSeconds has passed
	TickPassedDeltaTimeSeconds += InDeltaTime;
	if (TickPassedDeltaTimeSeconds < TickUpdateTimeSeconds)
	{
		return;
	}

	// Update the value
	TickPassedDeltaTimeSeconds = 0.f;
	UpdateVariableNodeFromActor();
}

void SDlgDataPropertyValue::UpdateVariableNodeFromActor()
{
	if (!VariableNode.IsValid())
	{
		return;
	}

	TWeakObjectPtr<const AActor> Actor = VariableNode->GetParentActor();
	if (!Actor.IsValid())
	{
		return;
	}

	const FName VariableName = VariableNode->GetVariableName();
	switch (VariableNode->GetVariableType())
	{
		case EDlgDataDisplayVariableTreeNodeType::Integer:
		{
			const int32 Value = IDlgDialogueParticipant::Execute_GetIntValue(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(FString::FromInt(Value));
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::Float:
		{
			const float Value = IDlgDialogueParticipant::Execute_GetFloatValue(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(FString::SanitizeFloat(Value));
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::Bool:
		{
			const bool Value = IDlgDialogueParticipant::Execute_GetBoolValue(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(BoolToFString(Value));
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::FName:
		{
			const FName Value = IDlgDialogueParticipant::Execute_GetNameValue(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(Value.ToString());
			break;
		}

		case EDlgDataDisplayVariableTreeNodeType::ClassInteger:
		{
			const int32 Value = FNYReflectionHelper::GetVariable<FIntProperty, int32>(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(FString::FromInt(Value));
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::ClassFloat:
		{
			const float Value = FNYReflectionHelper::GetVariable<FFloatProperty, float>(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(FString::SanitizeFloat(Value));
			break;
		}
		{
		case EDlgDataDisplayVariableTreeNodeType::ClassBool:
			const bool Value = FNYReflectionHelper::GetVariable<FBoolProperty, bool>(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(BoolToFString(Value));
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::ClassFName:
		{
			const FName Value = FNYReflectionHelper::GetVariable<FNameProperty, FName>(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(Value.ToString());
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::ClassFText:
		{
			const FText Value = FNYReflectionHelper::GetVariable<FTextProperty, FText>(Actor.Get(), VariableName);
			VariableNode->SetVariableValue(Value.ToString());
			break;
		}

		case EDlgDataDisplayVariableTreeNodeType::Event:
		{
			// Event does not have any state value, ignore
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::Condition:
		{
			const bool Value = IDlgDialogueParticipant::Execute_CheckCondition(Actor.Get(), nullptr, VariableName);
			VariableNode->SetVariableValue(BoolToFString(Value));
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::Default:
		default:
			VariableNode->SetVariableValue(TEXT("UNIMPLEMENTED - SHOULD NEVER HAPPEN"));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataTextPropertyValue
void SDlgDataTextPropertyValue::Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode)
{
	VariableNode = InVariableNode;
	if (!VariableNode.IsValid())
	{
		return;
	}

	UpdateVariableNodeFromActor();
	bIsFNameProperty = VariableNode->GetVariableType() == EDlgDataDisplayVariableTreeNodeType::FName;

	ChildSlot
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SAssignNew(TextBoxWidget, SEditableTextBox)
			.Text(this, &Self::GetTextValue)
			.SelectAllTextWhenFocused(true)
			.ClearKeyboardFocusOnCommit(false)
			.SelectAllTextOnCommit(true)
			.MinDesiredWidth(120.f)
			.OnTextCommitted(this, &Self::HandleTextCommitted)
			.OnTextChanged(this, &Self::HandleTextChanged)
			.IsReadOnly(this, &Self::IsReadOnly)
		]
	];
	PrimaryWidget = TextBoxWidget;
}

void SDlgDataTextPropertyValue::HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	static const FString MultipleValues(TEXT("Multiple Values"));
	const FString NewString = NewText.ToString();
	if (NewString == MultipleValues || !VariableNode.IsValid())
	{
		// can't set this :(
		return;
	}

	TWeakObjectPtr<AActor> Actor = VariableNode->GetParentActor();
	if (!Actor.IsValid())
	{
		return;
	}

	const FName VariableName = VariableNode->GetVariableName();
	switch (VariableNode->GetVariableType())
	{
		case EDlgDataDisplayVariableTreeNodeType::Integer:
		{
			const int32 Value = NewString.IsNumeric() ? FCString::Atoi(*NewString) : 0;
			IDlgDialogueParticipant::Execute_ModifyIntValue(Actor.Get(), VariableName, false, Value);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::Float:
		{
			const float Value = NewString.IsNumeric() ? FCString::Atof(*NewString) : 0.f;
			IDlgDialogueParticipant::Execute_ModifyFloatValue(Actor.Get(), VariableName, false, Value);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::Bool:
		{
			const bool Value = FStringToBool(NewString);
			IDlgDialogueParticipant::Execute_ModifyBoolValue(Actor.Get(), VariableName, Value);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::FName:
		{
			const FName Value(*NewString);
			IDlgDialogueParticipant::Execute_ModifyNameValue(Actor.Get(), VariableName, Value);
			break;
		}

		case EDlgDataDisplayVariableTreeNodeType::ClassInteger:
		{
			const int32 Value = NewString.IsNumeric() ? FCString::Atoi(*NewString) : 0;
			FNYReflectionHelper::SetVariable<FIntProperty>(Actor.Get(), VariableName, Value);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::ClassFloat:
		{
			const float Value = NewString.IsNumeric() ? FCString::Atof(*NewString) : 0.f;
			FNYReflectionHelper::SetVariable<FFloatProperty>(Actor.Get(), VariableName, Value);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::ClassBool:
		{
			const bool Value = FStringToBool(NewString);
			FNYReflectionHelper::SetVariable<FBoolProperty>(Actor.Get(), VariableName, Value);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::ClassFName:
		{
			const FName Value(*NewString);
			FNYReflectionHelper::SetVariable<FNameProperty>(Actor.Get(), VariableName, Value);
			break;
		}

		case EDlgDataDisplayVariableTreeNodeType::ClassFText:
		{
			const FText Value = FText::FromString(NewString);
			FNYReflectionHelper::SetVariable<FTextProperty>(Actor.Get(), VariableName, Value);
			break;
		}

		// The remaining variable types do not make sense to be text
		case EDlgDataDisplayVariableTreeNodeType::Event:
		case EDlgDataDisplayVariableTreeNodeType::Condition:
		case EDlgDataDisplayVariableTreeNodeType::Default:
		default:
			break;
	}

	UpdateVariableNodeFromActor();
}

void SDlgDataTextPropertyValue::HandleTextChanged(const FText& NewText)
{
	if (bIsFNameProperty)
	{
		const FText ErrorMessage = ValidateNameLength(NewText);
		if (!ErrorMessage.IsEmpty())
		{
			VariableNode->SetVariableValue(ErrorMessage.ToString());
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataEventPropertyValue
void SDlgDataEventPropertyValue::Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode)
{
	VariableNode = InVariableNode;
	if (!VariableNode.IsValid())
	{
		return;
	}

	if (VariableNode->GetVariableType() == EDlgDataDisplayVariableTreeNodeType::Event)
	{
		ChildSlot
		[
			SAssignNew(PrimaryWidget, SButton)
			.ToolTipText(LOCTEXT("EventValueTooltipKey", "Triggers this event. Calls OnDialogueEvent on the Actor."))
			.OnClicked(this, &Self::HandleTriggerEventClicked)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EVentButtonTextKey", "Trigger Event"))
			]
		];
	}
	else
	{
		ChildSlot
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SDlgDataEventPropertyValueINVALID", "INVALID VariableType INVALID"))
		];
	}
}

FReply SDlgDataEventPropertyValue::HandleTriggerEventClicked()
{
	if (!VariableNode.IsValid() || VariableNode->GetVariableType() != EDlgDataDisplayVariableTreeNodeType::Event)
	{
		return FReply::Unhandled();
	}

	TWeakObjectPtr<AActor> Actor = VariableNode->GetParentActor();
	if (!Actor.IsValid())
	{
		return FReply::Unhandled();
	}

	// Trigger the event
	const FName EventName = VariableNode->GetVariableName();
	IDlgDialogueParticipant::Execute_OnDialogueEvent(Actor.Get(), nullptr, EventName);

	return FReply::Handled();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataBoolPropertyValue
void SDlgDataBoolPropertyValue::Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode)
{
	VariableNode = InVariableNode;
	if (!VariableNode.IsValid())
	{
		return;
	}

	UpdateVariableNodeFromActor();
	ChildSlot
	[
		SAssignNew(CheckBoxWidget, SCheckBox)
		.OnCheckStateChanged(this, &Self::HandleCheckStateChanged)
		.IsChecked(this, &Self::IsChecked)
		.Padding(0.0f)
	];
	PrimaryWidget = CheckBoxWidget;
	SetEnabled(TAttribute<bool>(this, &Self::IsBoolProperty));
}

bool SDlgDataBoolPropertyValue::HasKeyboardFocus() const
{
	// The rest of the focus methods are handled in the parent class.
	return CheckBoxWidget->HasKeyboardFocus();
}

FReply SDlgDataBoolPropertyValue::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// toggle the our checkbox
		CheckBoxWidget->ToggleCheckedState();

		// Set focus to this object, but don't capture the mouse
		return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	return FReply::Unhandled();
}

ECheckBoxState SDlgDataBoolPropertyValue::IsChecked() const
{
	if (!VariableNode.IsValid())
	{
		return ECheckBoxState::Undetermined;
	}

	const bool Value = FStringToBool(VariableNode->GetVariableValue());
	return Value ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SDlgDataBoolPropertyValue::HandleCheckStateChanged(ECheckBoxState InNewState)
{
	// Can only modify bool values
	if (!IsBoolProperty())
	{
		return;
	}

	TWeakObjectPtr<AActor> Actor = VariableNode->GetParentActor();
	if (!Actor.IsValid())
	{
		return;
	}

	// Set the bool value
	const FName VariableName = VariableNode->GetVariableName();
	const bool Value = InNewState == ECheckBoxState::Checked || InNewState == ECheckBoxState::Undetermined;
	if (VariableNode->GetVariableType() == EDlgDataDisplayVariableTreeNodeType::ClassBool)
	{
		FNYReflectionHelper::SetVariable<FBoolProperty>(Actor.Get(), VariableName, Value);
	}
	else
	{
		IDlgDialogueParticipant::Execute_ModifyBoolValue(Actor.Get(), VariableName, Value);
	}
	UpdateVariableNodeFromActor();
}

#undef LOCTEXT_NAMESPACE
