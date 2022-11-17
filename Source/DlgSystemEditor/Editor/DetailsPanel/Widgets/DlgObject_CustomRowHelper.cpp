// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgObject_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgEditableTextPropertyHandle.h"
#include "Editor.h"
#include "DlgSystemEditor/Editor/DetailsPanel/DlgDetailsPanelUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Images/SImage.h"
#include "K2Node_Event.h"
#include "SourceCodeNavigation.h"

#define LOCTEXT_NAMESPACE "DialogueObject_CustomRowHelper"
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgObject_CustomRowHelper
FDlgObject_CustomRowHelper::FDlgObject_CustomRowHelper(IDetailPropertyRow* InPropertyRow) :
	PropertyRow(InPropertyRow)
{
}

void FDlgObject_CustomRowHelper::Update()
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
		.ButtonStyle(FNYAppStyle::Get(), "HoverHintOnly")
		.ToolTipText(this, &Self::GetBrowseObjectText)
		.ContentPadding(4.f)
		.ForegroundColor(FSlateColor::UseForeground())
		.Visibility(this, &Self::GetBrowseButtonVisibility)
		.OnClicked(this, &Self::OnBrowseClicked)
		[
			SNew(SImage)
			.Image(FNYAppStyle::GetBrush("PropertyWindow.Button_Browse"))
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
		.ButtonStyle(FNYAppStyle::Get(), "HoverHintOnly")
		.ToolTipText(this, &Self::GetJumpToObjectText)
		.ContentPadding(4.f)
		.ForegroundColor(FSlateColor::UseForeground())
		.Visibility(this, &Self::GetOpenButtonVisibility)
		.OnClicked(this, &Self::OnOpenClicked)
		[
			SNew(SImage)
			 .Image(FNYAppStyle::GetBrush("PropertyWindow.Button_Edit"))
			 .ColorAndOpacity( FSlateColor::UseForeground() )

			// SNew(STextBlock)
			// .Text(LOCTEXT("OpenObjectKey", "Open"))
			// .Font(DEFAULT_FONT("Regular", 10))
		]
	];
}

UObject* FDlgObject_CustomRowHelper::GetObject() const
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

UBlueprint* FDlgObject_CustomRowHelper::GetBlueprint() const
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


bool FDlgObject_CustomRowHelper::IsObjectABlueprint() const
{
	return GetBlueprint() != nullptr;
}

FReply FDlgObject_CustomRowHelper::OnBrowseClicked()
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

FReply FDlgObject_CustomRowHelper::OnOpenClicked()
{
	if (UBlueprint* Blueprint = GetBlueprint())
	{
		FDlgEditorUtilities::OpenBlueprintEditor(
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

FText FDlgObject_CustomRowHelper::GetBrowseObjectText() const
{
	return LOCTEXT("BrowseButtonToolTipText", "Browse to Asset in Content Browser");
}

FText FDlgObject_CustomRowHelper::GetJumpToObjectText() const
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

EVisibility FDlgObject_CustomRowHelper::GetOpenButtonVisibility() const
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

EVisibility FDlgObject_CustomRowHelper::GetBrowseButtonVisibility() const
{
	if (!CanBeVisible())
	{
		return EVisibility::Collapsed;
	}

	return IsObjectABlueprint() ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
#undef DEFAULT_FONT
