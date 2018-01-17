// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "TextPropertyPickList_CustomRowHelper.h"

#include "DetailWidgetRow.h"

#include "DialogueEditor/DetailsPanel/STextPropertyPickList.h"

#define LOCTEXT_NAMESPACE "TextPropertyPickList_CustomRowHelper"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FTextPropertyPickList_CustomRowHelper
void FTextPropertyPickList_CustomRowHelper::UpdateInternal()
{
	check(TextPropertyPickListWidget.IsValid());
	TextPropertyPickListWidget->SetToolTipAttribute(ToolTip);
	TextPropertyPickListWidget->SetPropertyHandle(PropertyHandle);

	DetailWidgetRow
	->NameContent()
	[
		NameContentWidget.ToSharedRef()
	]
	.ValueContent()
	// Similar to TextProperty, see FTextCustomization
	.MinDesiredWidth(209.f)
	.MaxDesiredWidth(600.f)
	[
		TextPropertyPickListWidget.ToSharedRef()
	];
}

#undef LOCTEXT_NAMESPACE
