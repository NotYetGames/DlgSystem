// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DetailWidgetRow.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DlgBase_CustomRowHelper.h"
#include "SDlgTextPropertyEditableTextBox.h"

class FDetailWidgetRow;
class UDlgDialogue;

// Custom row for integers
class DLGSYSTEMEDITOR_API FDlgIntTextBox_CustomRowHelper :
	public FDlgBase_CustomRowHelper,
	public TSharedFromThis<FDlgIntTextBox_CustomRowHelper>
{
	typedef FDlgIntTextBox_CustomRowHelper Self;
	typedef FDlgBase_CustomRowHelper Super;
public:
	FDlgIntTextBox_CustomRowHelper(
		FDetailWidgetRow* InDetailWidgetRow,
		const TSharedPtr<IPropertyHandle>& InPropertyHandle,
		const UDlgDialogue* InDialogue
	) : FDlgBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle), Dialogue(InDialogue) {}

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

	TWeakObjectPtr<const UDlgDialogue> Dialogue = nullptr;
	TAttribute<EVisibility> JumpToNodeVisibility;
};
