// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueK2Node_SwitchDialogueCallbackVariants.h"

UDialogueK2Node_SwitchDialogueCallbackEvent::UDialogueK2Node_SwitchDialogueCallbackEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CallbackType = EDlgDialogueCallback::Event;
}

FText UDialogueK2Node_SwitchDialogueCallbackEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "Switch_DialogueCallbackEvent", "Switch on Relevant Dialogue Event");
}

FText UDialogueK2Node_SwitchDialogueCallbackEvent::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "SwitchDialogueCallbackEvent_ToolTip", "Lists all available events from all dialogues for the owner based on IDlgDialogueParticipant::GetParticipantName() function call");
}

UDialogueK2Node_SwitchDialogueCallbackCondition::UDialogueK2Node_SwitchDialogueCallbackCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CallbackType = EDlgDialogueCallback::Condition;
}

FText UDialogueK2Node_SwitchDialogueCallbackCondition::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "Switch_DialogueCallbackCondition", "Switch on Relevant Dialogue Condition");
}

FText UDialogueK2Node_SwitchDialogueCallbackCondition::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "SwitchDialogueCallbackCondition_ToolTip", "Lists all available conditions from all dialogues for the owner based on IDlgDialogueParticipant::GetParticipantName() function call");
}


UDialogueK2Node_SwitchDialogueCallbackFloatValue::UDialogueK2Node_SwitchDialogueCallbackFloatValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CallbackType = EDlgDialogueCallback::FloatValue;
}

FText UDialogueK2Node_SwitchDialogueCallbackFloatValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "Switch_DialogueFloatValue", "Switch on Relevant Dialogue Float Value");
}

FText UDialogueK2Node_SwitchDialogueCallbackFloatValue::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "SwitchDialogueFloatValue_ToolTip", "Lists all available float value name from all dialogues for the owner based on IDlgDialogueParticipant::GetParticipantName() function call");
}


UDialogueK2Node_SwitchDialogueCallbackIntValue::UDialogueK2Node_SwitchDialogueCallbackIntValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CallbackType = EDlgDialogueCallback::IntValue;
}

FText UDialogueK2Node_SwitchDialogueCallbackIntValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "Switch_DialogueIntValue", "Switch on Relevant Dialogue Int Value");
}

FText UDialogueK2Node_SwitchDialogueCallbackIntValue::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "SwitchDialogueIntValue_ToolTip", "Lists all available Integer value names from all dialogues for the owner based on IDlgDialogueParticipant::GetParticipantName() function call");
}


UDialogueK2Node_SwitchDialogueCallbackBoolValue::UDialogueK2Node_SwitchDialogueCallbackBoolValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CallbackType = EDlgDialogueCallback::BoolValue;
}

FText UDialogueK2Node_SwitchDialogueCallbackBoolValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "Switch_DialogueBoolValue", "Switch on Relevant Dialogue Bool Value");
}

FText UDialogueK2Node_SwitchDialogueCallbackBoolValue::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "SwitchDialogueBoolValue_ToolTip", "Lists all available Bool value names from all dialogues for the owner based on IDlgDialogueParticipant::GetParticipantName() function call");
}


UDialogueK2Node_SwitchDialogueCallbackNameValue::UDialogueK2Node_SwitchDialogueCallbackNameValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CallbackType = EDlgDialogueCallback::NameValue;
}

FText UDialogueK2Node_SwitchDialogueCallbackNameValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "Switch_DialogueNameValue", "Switch on Relevant Dialogue Name Value");
}

FText UDialogueK2Node_SwitchDialogueCallbackNameValue::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "SwitchDialogueNameValue_ToolTip", "Lists all available Name value names from all dialogues for the owner based on IDlgDialogueParticipant::GetParticipantName() function call");
}
