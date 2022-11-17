// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgTextPropertyPickList_CustomRowHelper.h"

#include "DetailWidgetRow.h"

#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/SDlgTextPropertyPickList.h"

#define LOCTEXT_NAMESPACE "TextPropertyPickList_CustomRowHelper"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTextPropertyPickList_CustomRowHelper
void FDlgTextPropertyPickList_CustomRowHelper::UpdateInternal()
{
	check(TextPropertyPickListWidget.IsValid());
	TextPropertyPickListWidget->SetToolTipAttribute(ToolTip);
	TextPropertyPickListWidget->SetPropertyHandle(PropertyHandle);

	DetailWidgetRow
	->NameContent()
	[
		NameContentWidget.ToSharedRef()
	];

	if (ParentStructPropertyHandle && ParentStructPropertyHandle->IsValidHandle())
	{
		DetailWidgetRow
		->ValueContent()
		// Similar to TextProperty, see FTextCustomization
		.MinDesiredWidth(209.f)
		.MaxDesiredWidth(600.f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				TextPropertyPickListWidget.ToSharedRef()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				ParentStructPropertyHandle->CreateDefaultPropertyButtonWidgets()
			]
		];
	}
	else
	{
		DetailWidgetRow
		->ValueContent()
		// Similar to TextProperty, see FTextCustomization
		.MinDesiredWidth(209.f)
		.MaxDesiredWidth(600.f)
		[
			TextPropertyPickListWidget.ToSharedRef()
		];
	}
}

#undef LOCTEXT_NAMESPACE
