// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "DlgNodeCondition.h"
#include "UObject/Object.h"


#include "DlgDialogue.h"


UDlgNodeCondition::UDlgNodeCondition()
{
	
}



bool UDlgNodeCondition::EnterCondition_Implementation(APlayerController* PlayerController, UObject* Participant)
{
	return false;
}
