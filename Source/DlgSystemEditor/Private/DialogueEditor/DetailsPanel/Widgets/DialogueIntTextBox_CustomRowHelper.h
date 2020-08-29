// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DetailWidgetRow.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DialogueBase_CustomRowHelper.h"
#include "SDialogueTextPropertyEditableTextBox.h"

class FDetailWidgetRow;
class UDlgDialogue;

// Custom row for integers
class FDialogueIntTextBox_CustomRowHelper :
	public FDialogueBase_CustomRowHelper,
	public TSharedFromThis<FDialogueIntTextBox_CustomRowHelper>
{
	typedef FDialogueIntTextBox_CustomRowHelper Self;
	typedef FDialogueBase_CustomRowHelper Super;
public:
	FDialogueIntTextBox_CustomRowHelper(
		FDetailWidgetRow* InDetailWidgetRow,
		const TSharedPtr<IPropertyHandle>& InPropertyHandle,
		const UDlgDialogue* InDialogue
	) : FDialogueBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle), Dialogue(InDialogue) {}

	Self& SetJumpToNodeVisibility(const TAttribute<EVisibility>& Visibility)
	{
		JumpToNodeVisibility = Visibility;
		return *this;
	}

protected:
	void UpdateInternal() override;

	// Reset to default
	FText GetResetToolTip() const;
	EVisibility GetDiffersFromDefaultAsVisibility() const;
	FReply OnResetClicked();
	FReply OnJumpToNodeClicked();

private:
	bool bAddResetToDefaultWidget = true;

	TWeakObjectPtr<UDlgDialogue> Dialogue = nullptr;

	TAttribute<EVisibility> JumpToNodeVisibility;
};
