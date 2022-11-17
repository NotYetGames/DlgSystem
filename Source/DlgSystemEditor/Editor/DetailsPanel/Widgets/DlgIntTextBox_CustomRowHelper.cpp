// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgIntTextBox_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgEditableTextPropertyHandle.h"
#include "DlgSystemEditor/Editor/DetailsPanel/DlgDetailsPanelUtils.h"

#define LOCTEXT_NAMESPACE "DialogueIntTextBox_CustomRowHelper"
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgMultiLineEditableTextBox_CustomRowHelper
void FDlgIntTextBox_CustomRowHelper::UpdateInternal()
{
	check(PropertyHandle.IsValid());

	TSharedPtr<SHorizontalBox> HorizontalBox;
	DetailWidgetRow
	->NameContent()
	[
		NameContentWidget.ToSharedRef()
	]
	.ValueContent()
	// Similar to TextProperty, see FTextCustomization
	.MinDesiredWidth(200.f)
	// .MaxDesiredWidth(300.f)
	[
		SAssignNew(HorizontalBox, SHorizontalBox)
		+SHorizontalBox::Slot()
		.Padding(0.f, 0.f, 4.f, 0.f)
		.FillWidth(1.f)
		[
			PropertyHandle->CreatePropertyValueWidget()
		]
	];

	// Add Reset to default
	if (bAddResetToDefaultWidget)
	{
		PropertyHandle->MarkResetToDefaultCustomized(true);
		HorizontalBox->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.f, 2.f)
		[
			SNew(SButton)
			.IsFocusable(false)
			.ToolTipText(this, &Self::GetResetToolTip)
			.ButtonStyle(FNYAppStyle::Get(), "NoBorder")
			.ContentPadding(0)
			.Visibility(this, &Self::GetDiffersFromDefaultAsVisibility)
			.OnClicked(this, &Self::OnResetClicked)
			.Content()
			[
				SNew(SImage)
				.Image(FNYAppStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
			]
		];
	}

	// Jump to Node
	HorizontalBox->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(4.f, 2.f)
	[
		SNew(SButton)
		.Visibility(JumpToNodeVisibility)
		.ToolTipText(LOCTEXT("JumpToNodeTooltipKey", "Jump to Node"))
		.OnClicked(this, &Self::OnJumpToNodeClicked)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("JumpToNodeKey", "Jump to Node"))
			.Font(DEFAULT_FONT("Regular", 10))
		]
	];
}

FText FDlgIntTextBox_CustomRowHelper::GetResetToolTip() const
{
	FString Tooltip = NSLOCTEXT("PropertyEditor", "ResetToDefaultToolTip", "Reset to Default").ToString();
	if (PropertyHandle.IsValid() && !PropertyHandle->IsEditConst() && PropertyHandle->DiffersFromDefault())
	{
		const FString DefaultLabel = PropertyHandle->GetResetToDefaultLabel().ToString();
		if (DefaultLabel.Len() > 0)
		{
			Tooltip += "\n";
			Tooltip += DefaultLabel;
		}
	}

	return FText::FromString(Tooltip);
}

EVisibility FDlgIntTextBox_CustomRowHelper::GetDiffersFromDefaultAsVisibility() const
{
	if (PropertyHandle.IsValid())
	{
		return PropertyHandle->DiffersFromDefault() ? EVisibility::Visible : EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

FReply FDlgIntTextBox_CustomRowHelper::OnResetClicked()
{
	if (PropertyHandle.IsValid())
	{
		PropertyHandle->ResetToDefault();
	}
	return FReply::Handled();
}

FReply FDlgIntTextBox_CustomRowHelper::OnJumpToNodeClicked()
{
	int32 NodeIndex = 0;
	if (PropertyHandle->GetValue(NodeIndex) == FPropertyAccess::Success)
	{
		FDlgEditorUtilities::JumpToGraphNodeIndex(Dialogue.Get(), NodeIndex);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
#undef DEFAULT_FONT
