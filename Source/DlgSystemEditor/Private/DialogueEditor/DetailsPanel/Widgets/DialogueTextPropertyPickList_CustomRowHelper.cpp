// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueTextPropertyPickList_CustomRowHelper.h"

#include "DetailWidgetRow.h"

#include "DialogueEditor/DetailsPanel/Widgets/SDialogueTextPropertyPickList.h"

#define LOCTEXT_NAMESPACE "TextPropertyPickList_CustomRowHelper"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueTextPropertyPickList_CustomRowHelper
void FDialogueTextPropertyPickList_CustomRowHelper::UpdateInternal()
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
