// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

#include "DialogueBrowserTreeVariableProperties.h"
#include "TreeViewHelpers/DlgTreeViewParticipantProperties.h"

/** Used as a key in the fast lookup table. */
class FDialogueBrowserTreeParticipantProperties : public FDlgTreeViewParticipantProperties<FDialogueBrowserTreeVariableProperties>
{
	typedef FDialogueBrowserTreeParticipantProperties Self;
	typedef FDlgTreeViewParticipantProperties Super;

public:
	FDialogueBrowserTreeParticipantProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>& InDialogues) : Super(InDialogues) {}
	FDialogueBrowserTreeParticipantProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>&& InDialogues) : Super(InDialogues) {}
};
