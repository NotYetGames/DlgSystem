// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DetailWidgetRow.h"

#include "DlgBase_CustomRowHelper.h"

class SDlgTextPropertyPickList;
class FDetailWidgetRow;

// Helper for details panel, when we want to use SDlgTextPropertyPickList in a custom row in the details panel
class DLGSYSTEMEDITOR_API FDlgTextPropertyPickList_CustomRowHelper : public FDlgBase_CustomRowHelper
{
	typedef FDlgTextPropertyPickList_CustomRowHelper Self;
	typedef FDlgBase_CustomRowHelper Super;
public:
	FDlgTextPropertyPickList_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, const TSharedPtr<IPropertyHandle>& InPropertyHandle)
		: FDlgBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle) {}

	// Set the SPropertyPickList
	Self& SetTextPropertyPickListWidget(const TSharedRef<SDlgTextPropertyPickList>& InWidget)
	{
		TextPropertyPickListWidget = InWidget;
		return *this;
	}

	// Call this before Update is called to have the default buttons (like array add/remove/duplicate) added next to the row
	// See FDlgParticipantName_Details for an example
	void SetParentStructPropertyHandle(const TSharedRef<IPropertyHandle>& InParentStructPropertyHandle) { ParentStructPropertyHandle = InParentStructPropertyHandle; }

private:
	void UpdateInternal() override;

private:
	// The TextPropertyPickList Widget.
	TSharedPtr<SDlgTextPropertyPickList> TextPropertyPickListWidget;


	// Optional struct widget for additional buttons for one liners, only used if set
	TSharedPtr<IPropertyHandle> ParentStructPropertyHandle;
};
