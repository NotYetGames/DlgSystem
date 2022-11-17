// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgNode_Start.h"

FString UDlgNode_Start::GetDesc()
{
	return TEXT("Possible entry point.\nDoes not have text, the first satisfied child is picked if there is any.\nStart nodes are evaluated from left to right.");
}
