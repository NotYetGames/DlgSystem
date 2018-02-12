// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "SDlgDataPropertyValues.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SMissingWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataProperty
void SDlgDataPropertyValue::Construct(const FArguments& InArgs, TSharedPtr<FDlgDataDisplayTreeVariableNode> InVariableNode)
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

void SDlgDataPropertyValue::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
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
			const FString ValueString = Value ? TEXT("True") : TEXT("False");
			VariableNode->SetVariableValue(ValueString);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::FName:
		{
			const FName Value = IDlgDialogueParticipant::Execute_GetNameValue(Actor.Get(), VariableName);
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
			// TODO
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::Default:
		default:
			VariableNode->SetVariableValue(TEXT("UNIMPLEMENTED - SHOULD NEVER HAPPEN"));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataTextPropertyValue
void SDlgDataTextPropertyValue::Construct(const FArguments& InArgs, TSharedPtr<FDlgDataDisplayTreeVariableNode> InVariableNode)
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
			const bool Value = FCString::ToBool(*NewString);
			IDlgDialogueParticipant::Execute_ModifyBoolValue(Actor.Get(), VariableName, Value);
			break;
		}
		case EDlgDataDisplayVariableTreeNodeType::FName:
		{
			const FName Value(*NewString);
			IDlgDialogueParticipant::Execute_ModifyNameValue(Actor.Get(), VariableName, Value);
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
		FText ErrorMessage = ValidateNameLength(NewText);
		if (!ErrorMessage.IsEmpty())
		{
			VariableNode->SetVariableValue(ErrorMessage.ToString());
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataEventPropertyValue
void SDlgDataEventPropertyValue::Construct(const FArguments& InArgs, TSharedPtr<FDlgDataDisplayTreeVariableNode> InVariableNode)
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
			SNew(SButton)
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
	IDlgDialogueParticipant::Execute_OnDialogueEvent(Actor.Get(), EventName);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
