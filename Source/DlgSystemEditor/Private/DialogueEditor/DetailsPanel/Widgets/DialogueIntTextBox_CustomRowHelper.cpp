// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueIntTextBox_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "IPropertyUtilities.h"
#include "DialogueEditableTextPropertyHandle.h"
#include "DialogueEditor/DetailsPanel/DialogueDetailsPanelUtils.h"

#define LOCTEXT_NAMESPACE "DialogueIntTextBox_CustomRowHelper"
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueMultiLineEditableTextBox_CustomRowHelper
void FDialogueIntTextBox_CustomRowHelper::UpdateInternal()
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
			.ButtonStyle(FEditorStyle::Get(), "NoBorder")
			.ContentPadding(0)
			.Visibility(this, &Self::GetDiffersFromDefaultAsVisibility)
			.OnClicked(this, &Self::OnResetClicked)
			.Content()
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
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

FText FDialogueIntTextBox_CustomRowHelper::GetResetToolTip() const
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

EVisibility FDialogueIntTextBox_CustomRowHelper::GetDiffersFromDefaultAsVisibility() const
{
	if (PropertyHandle.IsValid())
	{
		return PropertyHandle->DiffersFromDefault() ? EVisibility::Visible : EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

FReply FDialogueIntTextBox_CustomRowHelper::OnResetClicked()
{
	if (PropertyHandle.IsValid())
	{
		PropertyHandle->ResetToDefault();
	}
	return FReply::Handled();
}

FReply FDialogueIntTextBox_CustomRowHelper::OnJumpToNodeClicked()
{
	int32 NodeIndex = 0;
	if (PropertyHandle->GetValue(NodeIndex) == FPropertyAccess::Success)
	{
		FDialogueEditorUtilities::JumpToGraphNodeIndex(Dialogue.Get(), NodeIndex);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
#undef DEFAULT_FONT
