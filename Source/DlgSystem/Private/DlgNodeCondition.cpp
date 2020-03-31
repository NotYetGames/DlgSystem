// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "DlgNodeCondition.h"
#include "UObject/Object.h"

#include "DlgDialogue.h"

bool UDlgNodeCondition::IsConditionMet_Implementation(const UObject* Participant)
{
	return false;
}
