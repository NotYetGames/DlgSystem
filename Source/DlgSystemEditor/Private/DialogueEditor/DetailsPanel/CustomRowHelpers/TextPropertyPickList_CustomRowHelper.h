// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DetailWidgetRow.h"

#include "Base_CustomRowHelper.h"

class STextPropertyPickList;
class FDetailWidgetRow;

/**
 * Helper for details panel, when we want to use STextPropertyPickList in a custom row in the details panel
 */
class FTextPropertyPickList_CustomRowHelper : public FBase_CustomRowHelper
{
	typedef FTextPropertyPickList_CustomRowHelper Self;
	typedef FBase_CustomRowHelper Super;
public:
	FTextPropertyPickList_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, const TSharedPtr<IPropertyHandle>& InPropertyHandle)
		: FBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle) {}

	/** Set the SPropertyPickList */
	Self* SetTextPropertyPickListWidget(const TSharedRef<STextPropertyPickList>& InWidget)
	{
		TextPropertyPickListWidget = InWidget;
		return this;
	}

private:
	void UpdateInternal() override;

private:
	/** The TextPropertyPickList Widget. */
	TSharedPtr<STextPropertyPickList> TextPropertyPickListWidget;
};
