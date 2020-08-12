// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DetailWidgetRow.h"

#include "DialogueBase_CustomRowHelper.h"

class SDialogueTextPropertyPickList;
class FDetailWidgetRow;

// Helper for details panel, when we want to use SDialogueTextPropertyPickList in a custom row in the details panel
class FDialogueTextPropertyPickList_CustomRowHelper : public FDialogueBase_CustomRowHelper
{
	typedef FDialogueTextPropertyPickList_CustomRowHelper Self;
	typedef FDialogueBase_CustomRowHelper Super;
public:
	FDialogueTextPropertyPickList_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, const TSharedPtr<IPropertyHandle>& InPropertyHandle)
		: FDialogueBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle) {}

	// Set the SPropertyPickList
	Self& SetTextPropertyPickListWidget(const TSharedRef<SDialogueTextPropertyPickList>& InWidget)
	{
		TextPropertyPickListWidget = InWidget;
		return *this;
	}

private:
	void UpdateInternal() override;

private:
	// The TextPropertyPickList Widget.
	TSharedPtr<SDialogueTextPropertyPickList> TextPropertyPickListWidget;
};
