// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"

class FDetailWidgetRow;
class UDlgDialogue;
class UBlueprint;

// Custom row for Objects that most likely are Blueprints?
// This only works with the IDetailPropertyRow::CustomWidget
// If we don't use this the children of EditInlineNew won't be displayed
class FDialogueObject_CustomRowHelper : public TSharedFromThis<FDialogueObject_CustomRowHelper>
{
	typedef FDialogueObject_CustomRowHelper Self;

public:
	FDialogueObject_CustomRowHelper(IDetailPropertyRow* InPropertyRow);
	virtual ~FDialogueObject_CustomRowHelper() {}

	// Update the full property row.
	void Update();

protected:
	// Reset to default
	FReply OnBrowseClicked();
	FReply OnOpenClicked();

	UObject* GetObject() const;
	UBlueprint* GetBlueprint() const;

	EVisibility GetButtonsVisibility() const;

protected:
	bool bAddResetToDefaultWidget = true;

	// The Property handle of what this row represents
	IDetailPropertyRow* PropertyRow = nullptr;
};
