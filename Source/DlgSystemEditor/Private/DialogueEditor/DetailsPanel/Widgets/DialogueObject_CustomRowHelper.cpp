// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueObject_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "DialogueEditableTextPropertyHandle.h"
#include "Editor.h"
#include "DialogueEditor/DetailsPanel/DialogueDetailsPanelUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Images/SImage.h"
#include "K2Node_Event.h"
#include "SourceCodeNavigation.h"

#define LOCTEXT_NAMESPACE "DialogueObject_CustomRowHelper"
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueObject_CustomRowHelper
FDialogueObject_CustomRowHelper::FDialogueObject_CustomRowHelper(IDetailPropertyRow* InPropertyRow) :
	PropertyRow(InPropertyRow)
{
}

void FDialogueObject_CustomRowHelper::Update()
{
	// SPropertyEditorClass for Classes
	// SPropertyEditorAsset for EditInlineNew ?

	// Get Default widgets
	TSharedPtr<SWidget> DefaultNameWidget;
	TSharedPtr<SWidget> DefaultValueWidget;
	PropertyRow->GetDefaultWidgets(
		DefaultNameWidget,
		DefaultValueWidget,
		false
	);

	FDetailWidgetRow& DetailWidgetRow = PropertyRow->CustomWidget(true);
	DetailWidgetRow.NameContent()
	[
		DefaultNameWidget.ToSharedRef()
	];

	TSharedPtr<SHorizontalBox> HorizontalBox;
	DetailWidgetRow.ValueContent()
	.MinDesiredWidth(GetRowMinimumDesiredWidth())
	[
		SAssignNew(HorizontalBox, SHorizontalBox)
	];

	// Default previous widget
	 HorizontalBox->AddSlot()
	.Padding(0.f, 0.f, 2.f, 0.f)
	.FillWidth(1.f)
	// .AutoWidth()
	[
		DefaultValueWidget.ToSharedRef()
	];

	// Browse Asset
	HorizontalBox->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(4.f)
	[
		SNew(SButton)
		.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
		.ToolTipText(this, &Self::GetBrowseObjectText)
		.ContentPadding(4.f)
		.ForegroundColor(FSlateColor::UseForeground())
		.Visibility(this, &Self::GetBrowseButtonVisibility)
		.OnClicked(this, &Self::OnBrowseClicked)
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
			.ColorAndOpacity(FSlateColor::UseForeground())
		]
	];

	// Jump to Object
	HorizontalBox->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(4.f, 2.f)
	[
		SNew(SButton)
		.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
		.ToolTipText(this, &Self::GetJumpToObjectText)
		.ContentPadding(4.f)
		.ForegroundColor(FSlateColor::UseForeground())
		.Visibility(this, &Self::GetOpenButtonVisibility)
		.OnClicked(this, &Self::OnOpenClicked)
		[
			SNew(SImage)
			 .Image(FEditorStyle::GetBrush("PropertyWindow.Button_Edit"))
			 .ColorAndOpacity( FSlateColor::UseForeground() )

			// SNew(STextBlock)
			// .Text(LOCTEXT("OpenObjectKey", "Open"))
			// .Font(DEFAULT_FONT("Regular", 10))
		]
	];
}

UObject* FDialogueObject_CustomRowHelper::GetObject() const
{
	if (!PropertyRow)
	{
		return nullptr;
	}

	TSharedPtr<IPropertyHandle> PropertyHandle = PropertyRow->GetPropertyHandle();
	if (!PropertyHandle.IsValid())
	{
		return nullptr;
	}
	UObject* Object = nullptr;
	PropertyHandle->GetValue(Object);
	return Object;
}

UBlueprint* FDialogueObject_CustomRowHelper::GetBlueprint() const
{
	UObject* Object = GetObject();
	if (!Object)
	{
		return nullptr;
	}

	// Class
	UClass* Class = Object->GetClass();
	if (const UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(Class))
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintClass->ClassGeneratedBy))
		{
			return Blueprint;
		}
	}

	return Cast<UBlueprint>(Object);
}


bool FDialogueObject_CustomRowHelper::IsObjectABlueprint() const
{
	return GetBlueprint() != nullptr;
}

FReply FDialogueObject_CustomRowHelper::OnBrowseClicked()
{
	if (!GEditor)
	{
		return FReply::Handled();
	}

	static constexpr bool bFocusContentBrowser = true;
	TArray<UObject*> ObjectsToSyncTo;
	if (UBlueprint* Blueprint = GetBlueprint())
	{
		ObjectsToSyncTo.Add(Blueprint);
	}
	GEditor->SyncBrowserToObjects(ObjectsToSyncTo, bFocusContentBrowser);

	return FReply::Handled();
}

FReply FDialogueObject_CustomRowHelper::OnOpenClicked()
{
	if (UBlueprint* Blueprint = GetBlueprint())
	{
		FDialogueEditorUtilities::OpenBlueprintEditor(
			Blueprint,
			OpenType,
			FunctionNameToOpen,
			bForceFullEditor,
			bAddBlueprintFunctionIfItDoesNotExist
		);
	}
	else if (UObject* Object = GetObject())
	{
		// Native
		FSourceCodeNavigation::NavigateToClass(Object->GetClass());
	}

	return FReply::Handled();
}

FText FDialogueObject_CustomRowHelper::GetBrowseObjectText() const
{
	return LOCTEXT("BrowseButtonToolTipText", "Browse to Asset in Content Browser");
}

FText FDialogueObject_CustomRowHelper::GetJumpToObjectText() const
{
	if (IsObjectABlueprint())
	{
		return LOCTEXT("OpenObjectBlueprintTooltipKey", "Open Blueprint Editor");
	}

	// Native Class
	return FText::Format(
		LOCTEXT("OpenObjectBlueprintTooltipKey", "Open Source File in {0}"),
		FSourceCodeNavigation::GetSelectedSourceCodeIDE()
	);
}

EVisibility FDialogueObject_CustomRowHelper::GetOpenButtonVisibility() const
{
	if (!CanBeVisible())
	{
		return EVisibility::Collapsed;
	}

	if (UObject* Object = GetObject())
	{
		// Blueprint
		if (IsObjectABlueprint())
		{
			return EVisibility::Visible;
		}

		// Native
		return FSourceCodeNavigation::CanNavigateToClass(Object->GetClass()) ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return EVisibility::Collapsed;
}

EVisibility FDialogueObject_CustomRowHelper::GetBrowseButtonVisibility() const
{
	if (!CanBeVisible())
	{
		return EVisibility::Collapsed;
	}

	return IsObjectABlueprint() ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
#undef DEFAULT_FONT
