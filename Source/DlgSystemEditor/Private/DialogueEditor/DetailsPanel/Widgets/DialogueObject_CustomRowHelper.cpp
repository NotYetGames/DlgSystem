// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueObject_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "IPropertyUtilities.h"
#include "DialogueEditableTextPropertyHandle.h"
#include "Editor.h"
#include "DialogueEditor/DetailsPanel/DialogueDetailsPanelUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Images/SImage.h"

#define LOCTEXT_NAMESPACE "DialogueObject_CustomRowHelper"
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueMultiLineEditableTextBox_CustomRowHelper
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
		true
	);

	FDetailWidgetRow& DetailWidgetRow = PropertyRow->CustomWidget(true);

	DetailWidgetRow.NameContent()
	[
		DefaultNameWidget.ToSharedRef()
	];

	TSharedPtr<SHorizontalBox> HorizontalBox;
	DetailWidgetRow.ValueContent()
	.MinDesiredWidth(300.f)
	[
		SAssignNew(HorizontalBox, SHorizontalBox)
		+SHorizontalBox::Slot()
		.Padding(0.f, 0.f, 4.f, 0.f)
		.FillWidth(1.f)
		[
			DefaultValueWidget.ToSharedRef()
		]

		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.f)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			.ToolTipText(LOCTEXT( "BrowseButtonToolTipText", "Browse to Asset in Content Browser"))
			.ContentPadding(4.f)
			.ForegroundColor(FSlateColor::UseForeground())
			.Visibility(this, &Self::GetButtonsVisibility)
			.OnClicked(this, &Self::OnBrowseClicked)
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
				.ColorAndOpacity( FSlateColor::UseForeground() )
			]
		]

		// Jump to Object
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.f, 2.f)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			.ToolTipText(LOCTEXT("OpenObjectTooltipKey", "Open Asset in the Editor"))
			.ContentPadding(4.f)
			.ForegroundColor(FSlateColor::UseForeground())
			.Visibility(this, &Self::GetButtonsVisibility)
			.OnClicked(this, &Self::OnOpenClicked)
			[
				SNew(SImage)
				 .Image(FEditorStyle::GetBrush("PropertyWindow.Button_Edit"))
				 .ColorAndOpacity( FSlateColor::UseForeground() )

				// SNew(STextBlock)
				// .Text(LOCTEXT("OpenObjectKey", "Open"))
				// .Font(DEFAULT_FONT("Regular", 10))
			]
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

	return  Cast<UBlueprint>(Object);
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
		Blueprint->bForceFullEditor = bForceFullEditor;

		// Find Function Graph
		UObject* ObjectToFocusOn = nullptr;
		if (FunctionNameToOpen != NAME_None)
		{
			ObjectToFocusOn = FDialogueEditorUtilities::BlueprintGetOrAddFunction(Blueprint, FunctionNameToOpen, GetObject()->GetClass());
		}
		else if (EventNameToOpen != NAME_None)
		{
			ObjectToFocusOn = FDialogueEditorUtilities::BlueprintGetOrAddEvent(Blueprint, EventNameToOpen, GetObject()->GetClass());
		}

		// Default to the last uber graph
		if (ObjectToFocusOn == nullptr)
		{
			ObjectToFocusOn = Blueprint->GetLastEditedUberGraph();
		}
		if (ObjectToFocusOn)
		{
			FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(ObjectToFocusOn);
		}
		else
		{
			FDialogueEditorUtilities::OpenEditorForAsset(Blueprint);
		}
	}

	return FReply::Handled();
}

EVisibility FDialogueObject_CustomRowHelper::GetButtonsVisibility() const
{
	return GetObject() != nullptr ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
#undef DEFAULT_FONT
