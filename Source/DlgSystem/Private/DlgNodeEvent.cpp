// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "DlgNodeEvent.h"
#include "UObject/Object.h"


#include "DlgDialogue.h"


UDlgNodeEvent::UDlgNodeEvent()
{
	
}


void UDlgNodeEvent::EnterEvent_Implementation(APlayerController* PlayerController, UObject* Participant)
{
	return;
}
