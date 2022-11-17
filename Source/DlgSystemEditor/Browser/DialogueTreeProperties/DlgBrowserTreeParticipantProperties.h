// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

#include "DlgBrowserTreeVariableProperties.h"
#include "DlgSystem/TreeViewHelpers/DlgTreeViewParticipantProperties.h"

/** Used as a key in the fast lookup table. */
class FDlgBrowserTreeParticipantProperties : public FDlgTreeViewParticipantProperties<FDlgBrowserTreeVariableProperties>
{
	typedef FDlgBrowserTreeParticipantProperties Self;
	typedef FDlgTreeViewParticipantProperties Super;

public:
	FDlgBrowserTreeParticipantProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>& InDialogues) : Super(InDialogues) {}
	FDlgBrowserTreeParticipantProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>&& InDialogues) : Super(InDialogues) {}
};
