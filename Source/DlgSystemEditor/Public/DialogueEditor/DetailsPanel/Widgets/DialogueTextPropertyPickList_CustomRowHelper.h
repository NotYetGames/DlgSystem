// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DetailWidgetRow.h"

#include "DialogueBase_CustomRowHelper.h"

class SDialogueTextPropertyPickList;
class FDetailWidgetRow;

// Helper for details panel, when we want to use SDialogueTextPropertyPickList in a custom row in the details panel
class DLGSYSTEMEDITOR_API FDialogueTextPropertyPickList_CustomRowHelper : public FDialogueBase_CustomRowHelper
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

	// Call this before Update is called to have the default buttons (like array add/remove/duplicate) added next to the row
	// See FDialogueParticipantName_Details for an example
	void SetParentStructPropertyHandle(const TSharedRef<IPropertyHandle>& InParentStructPropertyHandle) { ParentStructPropertyHandle = InParentStructPropertyHandle; }

private:
	void UpdateInternal() override;

private:
	// The TextPropertyPickList Widget.
	TSharedPtr<SDialogueTextPropertyPickList> TextPropertyPickListWidget;


	// Optional struct widget for additional buttons for one liners, only used if set
	TSharedPtr<IPropertyHandle> ParentStructPropertyHandle;
};
