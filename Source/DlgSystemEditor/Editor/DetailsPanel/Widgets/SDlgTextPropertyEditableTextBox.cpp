// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgTextPropertyEditableTextBox.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"

#include "DlgSystem/DlgLocalizationHelper.h"
#include "DlgSystem/NYEngineVersionHelpers.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgEditableTextPropertyHandle.h"

#define LOCTEXT_NAMESPACE "STextPropertyEditableTextBox"

FText SDlgTextPropertyEditableTextBox::MultipleValuesText(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"));

void SDlgTextPropertyEditableTextBox::Construct(
	const FArguments& InArgs,
	const TSharedRef<FDlgEditableTextPropertyHandle>& InEditableTextProperty,
	const TSharedRef<IPropertyHandle>& InPropertyHandle
)
{
	EditableTextProperty = InEditableTextProperty;
	PropertyHandle = InPropertyHandle;
	bAddResetToDefaultWidget = InArgs._AddResetToDefaultWidget;

	TSharedPtr<SHorizontalBox> HorizontalBox;

	//const bool bIsPassword = EditableTextProperty->IsPassword();
	//bIsMultiLine = EditableTextProperty->IsMultiLineText();
	if (bIsMultiLine)
	{
		ChildSlot
		[
			SAssignNew(HorizontalBox, SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SBox)
				.MinDesiredWidth(InArgs._MinDesiredWidth)
				.MaxDesiredHeight(InArgs._MaxDesiredHeight)
				[
					SAssignNew(MultiLineWidget, SMultiLineEditableTextBox)
					.Text(this, &Self::GetTextValue)
					.ToolTipText(this, &Self::GetToolTipText)
					.Style(InArgs._Style)
					.Font(InArgs._Font)
					.ForegroundColor(InArgs._ForegroundColor)
					.ReadOnlyForegroundColor(InArgs._ReadOnlyForegroundColor)
					.SelectAllTextWhenFocused(InArgs._SelectAllTextWhenFocused)
					.ClearKeyboardFocusOnCommit(InArgs._ClearKeyboardFocusOnCommit)
					.SelectAllTextOnCommit(InArgs._SelectAllTextOnCommit)
					.OnTextChanged(this, &Self::OnTextChanged)
					.OnTextCommitted(this, &Self::OnTextCommitted)
					.IsReadOnly(this, &Self::IsSourceTextReadOnly)
					.AutoWrapText(InArgs._AutoWrapText)
					.WrapTextAt(InArgs._WrapTextAt)
					.ModiferKeyForNewLine(InArgs._ModiferKeyForNewLine)
					.AllowMultiLine(true)
					//.IsPassword(bIsPassword)
				]
			]
		];

		PrimaryWidget = MultiLineWidget;
	}
	else
	{
		checkNoEntry();
	}

	HorizontalBox->AddSlot()
		.AutoWidth()
		[
			SNew(SComboButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ContentPadding(FMargin(4, 0))
			.ButtonStyle(FNYAppStyle::Get(), "HoverHintOnly")
			.ForegroundColor(FSlateColor::UseForeground())
			.ToolTipText(LOCTEXT("AdvancedTextSettingsComboToolTip", "Edit advanced text settings."))
			.MenuContent()
			[
				SNew(SBox)
				.WidthOverride(340)
				.Padding(4)
				[
					SNew(SGridPanel)
					.FillColumn(1, 1.0f)

					// Inline Text
					+SGridPanel::Slot(0, 0)
					.ColumnSpan(2)
					.Padding(2)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FNYAppStyle::Get(), "LargeText")
						.Text(LOCTEXT("TextInlineTextLabel", "Inline Text"))
					]

					// Localizable?
					+SGridPanel::Slot(0, 1)
					.Padding(2)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TextLocalizableLabel", "Localizable:"))
					]
					+SGridPanel::Slot(1, 1)
					.Padding(2)
					[
						SNew(SHorizontalBox)

						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0)
						[
							SNew(SUniformGridPanel)
							.SlotPadding(FMargin(0, 0, 4, 0))

							+SUniformGridPanel::Slot(0, 0)
							[
								SNew(SCheckBox)
								.Style(FNYAppStyle::Get(), "ToggleButtonCheckbox")
								.ToolTipText(LOCTEXT("TextLocalizableToggleYesToolTip", "Assign this text a key and allow it to be gathered for localization."))
								.Padding(FMargin(4, 2))
								.HAlign(HAlign_Center)
								.IsEnabled(this, &Self::IsCultureInvariantFlagEnabled)
								.IsChecked(this, &Self::GetLocalizableCheckState, true/*bActiveState*/)
								.OnCheckStateChanged(this, &Self::HandleLocalizableCheckStateChanged, true/*bActiveState*/)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("TextLocalizableToggleYes", "Yes"))
								]
							]

							+SUniformGridPanel::Slot(1, 0)
							[
								SNew(SCheckBox)
								.Style(FNYAppStyle::Get(), "ToggleButtonCheckbox")
								.ToolTipText(LOCTEXT("TextLocalizableToggleNoToolTip", "Mark this text as 'culture invariant' to prevent it being gathered for localization."))
								.Padding(FMargin(4, 2))
								.HAlign(HAlign_Center)
								.IsEnabled(this, &Self::IsCultureInvariantFlagEnabled)
								.IsChecked(this, &Self::GetLocalizableCheckState, false/*bActiveState*/)
								.OnCheckStateChanged(this, &Self::HandleLocalizableCheckStateChanged, false/*bActiveState*/)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("TextLocalizableToggleNo", "No"))
								]
							]
						]
					]

#if USE_STABLE_LOCALIZATION_KEYS
					// Package
					+SGridPanel::Slot(0, 2)
					.Padding(2)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TextPackageLabel", "Package:"))
						.Visibility(this, &Self::GetLocalizableVisibility)
					]
					+SGridPanel::Slot(1, 2)
					.Padding(2)
					[
						SNew(SEditableTextBox)
						.Text(this, &Self::GetPackageValue)
						.Visibility(this, &Self::GetLocalizableVisibility)
						.IsReadOnly(true)
					]
#endif // USE_STABLE_LOCALIZATION_KEYS

					// Namespace
					+SGridPanel::Slot(0, 3)
					.Padding(2)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TextNamespaceLabel", "Namespace:"))
						.Visibility(this, &Self::GetLocalizableVisibility)
					]
					+SGridPanel::Slot(1, 3)
					.Padding(2)
					[
						SAssignNew(NamespaceEditableTextBox, SEditableTextBox)
						.Text(this, &Self::GetNamespaceValue)
						.SelectAllTextWhenFocused(true)
						.ClearKeyboardFocusOnCommit(false)
						.OnTextChanged(this, &Self::OnNamespaceChanged)
						.OnTextCommitted(this, &Self::OnNamespaceCommitted)
						.SelectAllTextOnCommit(true)
						.Visibility(this, &Self::GetLocalizableVisibility)
						.IsReadOnly(this, &Self::IsNamespaceReadOnly)
					]

					// Warning, key will be overriden
					+SGridPanel::Slot(2, 3)
					.Padding(2)
					[
						SNew(SImage)
						.Image(FCoreStyle::Get().GetBrush("Icons.Warning"))
						.Visibility(this, &Self::GetNamespaceOverridenWarningImageVisibility)
						.ToolTipText(LOCTEXT("NamespaceOverridenWarningToolTip", "The namespace will be overriden on Save.\nTo change this bevahiour go to Project Settings -> (Editors) Dialogue -> Localization"))
					]

					// Key
					+SGridPanel::Slot(0, 4)
					.Padding(2)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TextKeyLabel", "Key:"))
						.Visibility(this, &Self::GetLocalizableVisibility)
					]
					+SGridPanel::Slot(1, 4)
					.Padding(2)
					[
						SAssignNew(KeyEditableTextBox, SEditableTextBox)
						.Text(this, &Self::GetKeyValue)
						.Visibility(this, &Self::GetLocalizableVisibility)
#if USE_STABLE_LOCALIZATION_KEYS
						.SelectAllTextWhenFocused(true)
						.ClearKeyboardFocusOnCommit(false)
						.OnTextChanged(this, &Self::OnKeyChanged)
						.OnTextCommitted(this, &Self::OnKeyCommitted)
						.SelectAllTextOnCommit(true)
						.IsReadOnly(this, &Self::IsKeyReadOnly)
#else	// USE_STABLE_LOCALIZATION_KEYS
						.IsReadOnly(true)
#endif	// USE_STABLE_LOCALIZATION_KEYS
					]

					// Referenced Text
					+SGridPanel::Slot(0, 5)
					.ColumnSpan(2)
					.Padding(2)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FNYAppStyle::Get(), "LargeText")
						.Text(LOCTEXT("TextReferencedTextLabel", "Referenced Text"))
						.Visibility(this, &Self::GetLocalizableVisibility)
					]

					// String Table
					+SGridPanel::Slot(0, 6)
					.Padding(2)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TextStringTableLabel", "String Table:"))
						.Visibility(this, &Self::GetLocalizableVisibility)
					]
					+SGridPanel::Slot(1, 6)
					.Padding(2)
					[
						SNew(STextPropertyEditableStringTableReference, InEditableTextProperty)
						.AllowUnlink(true)
						.IsEnabled(this, &Self::CanEdit)
						.Visibility(this, &Self::GetLocalizableVisibility)
					]
				]
			]
		];

	HorizontalBox->AddSlot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FCoreStyle::Get().GetBrush("Icons.Warning"))
			.Visibility(this, &Self::GetTextWarningImageVisibility)
			.ToolTipText(LOCTEXT("TextNotLocalizedWarningToolTip", "This text is marked as 'culture invariant' and won't be gathered for localization.\nYou can change this by editing the advanced text settings."))
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

	SetEnabled(TAttribute<bool>(this, &Self::CanEdit));
}

FText SDlgTextPropertyEditableTextBox::GetResetToolTip() const
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

EVisibility SDlgTextPropertyEditableTextBox::GetDiffersFromDefaultAsVisibility() const
{
	if (PropertyHandle.IsValid())
	{
		return PropertyHandle->DiffersFromDefault() ? EVisibility::Visible : EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

FReply SDlgTextPropertyEditableTextBox::OnResetClicked()
{
	if (EditableTextProperty.IsValid() && PropertyHandle.IsValid())
	{
		PropertyHandle->ResetToDefault();
		SetTextValue(EditableTextProperty->GetText(0));
	}
	return FReply::Handled();
}


EVisibility SDlgTextPropertyEditableTextBox::GetLocalizableVisibility() const
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		return PropertyValue.IsCultureInvariant() ? EVisibility::Collapsed : EVisibility::Visible;
	}
	return EVisibility::Visible;
}

void SDlgTextPropertyEditableTextBox::GetDesiredWidth(float& OutMinDesiredWidth, float& OutMaxDesiredWidth)
{
	if (bIsMultiLine)
	{
		OutMinDesiredWidth = 250.0f;
	}
	else
	{
		OutMinDesiredWidth = 125.0f;
	}

	OutMaxDesiredWidth = 600.0f;
}

bool SDlgTextPropertyEditableTextBox::SupportsKeyboardFocus() const
{
	return PrimaryWidget.IsValid() && PrimaryWidget->SupportsKeyboardFocus();
}

FReply SDlgTextPropertyEditableTextBox::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	// Forward keyboard focus to our editable text widget
	return FReply::Handled().SetUserFocus(PrimaryWidget.ToSharedRef(), InFocusEvent.GetCause());
}

void SDlgTextPropertyEditableTextBox::Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime)
{
	const float CurrentHeight = AllottedGeometry.GetLocalSize().Y;
	if (bIsMultiLine && PreviousHeight.IsSet() && PreviousHeight.GetValue() != CurrentHeight)
	{
		EditableTextProperty->RequestRefresh();
	}
	PreviousHeight = CurrentHeight;
}

bool SDlgTextPropertyEditableTextBox::CanEdit() const
{
	const bool bIsReadOnly = FTextLocalizationManager::Get().IsLocalizationLocked() || EditableTextProperty->IsReadOnly();
	return !bIsReadOnly;
}

bool SDlgTextPropertyEditableTextBox::IsCultureInvariantFlagEnabled() const
{
	return !IsSourceTextReadOnly();
}

bool SDlgTextPropertyEditableTextBox::IsSourceTextReadOnly() const
{
	if (!CanEdit())
	{
		return true;
	}

	// We can't edit the source string of string table references
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText TextValue = EditableTextProperty->GetText(0);
		if (TextValue.IsFromStringTable())
		{
			return true;
		}
	}

	return false;
}

bool SDlgTextPropertyEditableTextBox::WillNamespaceBeUpdated() const
{
	return !IsIdentityReadOnly() && FDlgLocalizationHelper::WillTextNamespaceBeUpdated(GetTextValue());
}

bool SDlgTextPropertyEditableTextBox::IsNamespaceReadOnly() const
{
	return IsIdentityReadOnly() || WillNamespaceBeUpdated();
}

bool SDlgTextPropertyEditableTextBox::IsIdentityReadOnly() const
{
	if (!CanEdit())
	{
		return true;
	}

	// We can't edit the identity of texts that don't gather for localization
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText TextValue = EditableTextProperty->GetText(0);
		if (!TextValue.ShouldGatherForLocalization())
		{
			return true;
		}
	}

	return false;
}

FText SDlgTextPropertyEditableTextBox::GetToolTipText() const
{
	FText LocalizedTextToolTip;
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText TextValue = EditableTextProperty->GetText(0);

		if (TextValue.IsFromStringTable())
		{
			FName TableId;
			FString Key;

#if NY_ENGINE_VERSION >= 500
			FTextInspector::GetTableIdAndKey(TextValue, TableId, Key);
#else
			FStringTableRegistry::Get().FindTableIdAndKey(TextValue, TableId, Key);
#endif

			LocalizedTextToolTip = FText::Format(
				LOCTEXT("StringTableTextToolTipFmt", "--- String Table Reference ---\nTable ID: {0}\nKey: {1}"),
				FText::FromName(TableId), FText::FromString(Key)
				);
		}
		else
		{
			bool bIsLocalized = false;
			FString Namespace;
			FString Key;
			const FString* SourceString = FTextInspector::GetSourceString(TextValue);

			if (SourceString && TextValue.ShouldGatherForLocalization())
			{
#if NY_ENGINE_VERSION >= 500
				const FTextId TextId = FTextInspector::GetTextId(TextValue);
				bIsLocalized = !TextId.IsEmpty();
				if (bIsLocalized)
				{
					Namespace = TextId.GetNamespace().GetChars();
					Key = TextId.GetKey().GetChars();
				}
#else
				bIsLocalized = FTextLocalizationManager::Get().FindNamespaceAndKeyFromDisplayString(FTextInspector::GetSharedDisplayString(TextValue), Namespace, Key);
#endif

			}

			if (bIsLocalized)
			{
				const FString PackageNamespace = TextNamespaceUtil::ExtractPackageNamespace(Namespace);
				const FString TextNamespace = TextNamespaceUtil::StripPackageNamespace(Namespace);

				LocalizedTextToolTip = FText::Format(
					LOCTEXT("LocalizedTextToolTipFmt", "--- Localized Text ---\nPackage: {0}\nNamespace: {1}\nKey: {2}\nSource: {3}"),
					FText::FromString(PackageNamespace), FText::FromString(TextNamespace), FText::FromString(Key), FText::FromString(*SourceString)
					);
			}
		}
	}

	FText BaseToolTipText = EditableTextProperty->GetToolTipText();
	if (FTextLocalizationManager::Get().IsLocalizationLocked())
	{
		const FText LockdownToolTip = FTextLocalizationManager::Get().IsGameLocalizationPreviewEnabled()
			? LOCTEXT("LockdownToolTip_Preview", "Localization is locked down due to the active game localization preview")
			: LOCTEXT("LockdownToolTip_Other", "Localization is locked down");
		BaseToolTipText = BaseToolTipText.IsEmptyOrWhitespace() ? LockdownToolTip : FText::Format(LOCTEXT("ToolTipLockdownFmt", "!!! {0} !!!\n\n{1}"), LockdownToolTip, BaseToolTipText);
	}

	if (LocalizedTextToolTip.IsEmptyOrWhitespace())
	{
		return BaseToolTipText;
	}
	if (BaseToolTipText.IsEmptyOrWhitespace())
	{
		return LocalizedTextToolTip;
	}

	return FText::Format(LOCTEXT("ToolTipCompleteFmt", "{0}\n\n{1}"), BaseToolTipText, LocalizedTextToolTip);
}

FText SDlgTextPropertyEditableTextBox::GetTextValue() const
{
	FText TextValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		TextValue = EditableTextProperty->GetText(0);
	}
	else if (NumTexts > 1)
	{
		TextValue = MultipleValuesText;
	}

	return TextValue;
}

void SDlgTextPropertyEditableTextBox::SetTextValue(const FText& NewValue)
{
	if (MultiLineWidget.IsValid())
	{
		MultiLineWidget->SetText(NewValue);
	}
}

void SDlgTextPropertyEditableTextBox::OnTextChanged(const FText& NewText)
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	FText TextErrorMsg;

	// Don't validate the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || NewText.ToString().Equals(MultipleValuesText.ToString(), ESearchCase::CaseSensitive)))
	{
		EditableTextProperty->IsValidText(NewText, TextErrorMsg);
	}

	// Update or clear the error message
	SetTextError(TextErrorMsg);
	TextChangedEvent.Broadcast(NewText);
}

void SDlgTextPropertyEditableTextBox::OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	// Don't commit the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || !NewText.ToString().Equals(MultipleValuesText.ToString(), ESearchCase::CaseSensitive)))
	{
		FText TextErrorMsg;
		if (EditableTextProperty->IsValidText(NewText, TextErrorMsg))
		{
			// Valid text; clear any error
			SetTextError(FText::GetEmpty());
		}
		else
		{
			// Invalid text; set the error and prevent the new text from being set
			SetTextError(TextErrorMsg);
			return;
		}

		const FString& SourceString = NewText.ToString();
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Only apply the change if the new text is different
			if (PropertyValue.ToString().Equals(NewText.ToString(), ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Is the new text is empty, just use the empty instance
			if (NewText.IsEmpty())
			{
				EditableTextProperty->SetText(TextIndex, FText::GetEmpty());
				continue;
			}

			// Maintain culture invariance when editing the text
			if (PropertyValue.IsCultureInvariant())
			{
				EditableTextProperty->SetText(TextIndex, FText::AsCultureInvariant(NewText.ToString()));
				continue;
			}

			FString NewNamespace;
			FString NewKey;
#if USE_STABLE_LOCALIZATION_KEYS
			{
				// Get the stable namespace and key that we should use for this property
				const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
				EditableTextProperty->GetStableTextId(
					TextIndex,
					IEditableTextProperty::ETextPropertyEditAction::EditedSource,
					TextSource ? *TextSource : FString(),
					FTextInspector::GetNamespace(PropertyValue).Get(FString()),
					FTextInspector::GetKey(PropertyValue).Get(FString()),
					NewNamespace,
					NewKey
					);
			}
#else	// USE_STABLE_LOCALIZATION_KEYS
			{
				// We want to preserve the namespace set on this property if it's *not* the default value
				if (!EditableTextProperty->IsDefaultValue())
				{
					// Some properties report that they're not the default, but still haven't been set from a property, so we also check the property key to see if it's a valid GUID before allowing the namespace to persist
					FGuid TmpGUID;
					if (FGuid::Parse(FTextInspector::GetKey(PropertyValue).Get(FString()), TmpGUID))
					{
						NewNamespace = FTextInspector::GetNamespace(PropertyValue).Get(FString());
					}
				}

				NewKey = FGuid::NewGuid().ToString();
			}
#endif	// USE_STABLE_LOCALIZATION_KEYS

			const FText FinalText = FText::ChangeKey(NewNamespace, NewKey, NewText);
			EditableTextProperty->SetText(TextIndex, FinalText);
			TextCommittedEvent.Broadcast(FinalText, CommitInfo);
		}
	}
}

void SDlgTextPropertyEditableTextBox::SetTextError(const FText& InErrorMsg)
{
	if (MultiLineWidget.IsValid())
	{
		MultiLineWidget->SetError(InErrorMsg);
	}

	// if (SingleLineWidget.IsValid())
	// {
	// 	SingleLineWidget->SetError(InErrorMsg);
	// }
}

FText SDlgTextPropertyEditableTextBox::GetNamespaceValue() const
{
	FText NamespaceValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		TOptional<FString> FoundNamespace = FTextInspector::GetNamespace(PropertyValue);
		if (FoundNamespace.IsSet())
		{
			NamespaceValue = FText::FromString(TextNamespaceUtil::StripPackageNamespace(FoundNamespace.GetValue()));
		}
	}
	else if (NumTexts > 1)
	{
		NamespaceValue = MultipleValuesText;
	}

	return NamespaceValue;
}

void SDlgTextPropertyEditableTextBox::OnNamespaceChanged(const FText& NewText)
{
	FText ErrorMessage;
	const FText ErrorCtx = LOCTEXT("TextNamespaceErrorCtx", "Namespace");
	IsValidIdentity(NewText, &ErrorMessage, &ErrorCtx);

	NamespaceEditableTextBox->SetError(ErrorMessage);
}

void SDlgTextPropertyEditableTextBox::OnNamespaceCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	if (!IsValidIdentity(NewText))
	{
		return;
	}

	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	// Don't commit the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || NewText.ToString() != MultipleValuesText.ToString()))
	{
		const FString& TextNamespace = NewText.ToString();
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Only apply the change if the new namespace is different - we want to keep the keys stable where possible
			const FString CurrentTextNamespace = TextNamespaceUtil::StripPackageNamespace(FTextInspector::GetNamespace(PropertyValue).Get(FString()));
			if (CurrentTextNamespace.Equals(TextNamespace, ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Get the stable namespace and key that we should use for this property
			FString NewNamespace;
			FString NewKey;
#if USE_STABLE_LOCALIZATION_KEYS
			{
				const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
				EditableTextProperty->GetStableTextId(
					TextIndex,
					IEditableTextProperty::ETextPropertyEditAction::EditedNamespace,
					TextSource ? *TextSource : FString(),
					TextNamespace,
					FTextInspector::GetKey(PropertyValue).Get(FString()),
					NewNamespace,
					NewKey
					);
			}
#else	// USE_STABLE_LOCALIZATION_KEYS
			{
				NewNamespace = TextNamespace;

				// If the current key is a GUID, then we can preserve that when setting the new namespace
				NewKey = FTextInspector::GetKey(PropertyValue).Get(FString());
				{
					FGuid TmpGuid;
					if (!FGuid::Parse(NewKey, TmpGuid))
					{
						NewKey = FGuid::NewGuid().ToString();
					}
				}
			}
#endif	// USE_STABLE_LOCALIZATION_KEYS

			EditableTextProperty->SetText(TextIndex, FText::ChangeKey(NewNamespace, NewKey, PropertyValue));
		}
	}
}

FText SDlgTextPropertyEditableTextBox::GetKeyValue() const
{
	FText KeyValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		TOptional<FString> FoundKey = FTextInspector::GetKey(PropertyValue);
		if (FoundKey.IsSet())
		{
			KeyValue = FText::FromString(FoundKey.GetValue());
		}
	}
	else if (NumTexts > 1)
	{
		KeyValue = MultipleValuesText;
	}

	return KeyValue;
}

#if USE_STABLE_LOCALIZATION_KEYS

void SDlgTextPropertyEditableTextBox::OnKeyChanged(const FText& NewText)
{
	FText ErrorMessage;
	const FText ErrorCtx = LOCTEXT("TextKeyErrorCtx", "Key");
	const bool bIsValidName = IsValidIdentity(NewText, &ErrorMessage, &ErrorCtx);

	if (NewText.IsEmptyOrWhitespace())
	{
		ErrorMessage = LOCTEXT("TextKeyEmptyErrorMsg", "Key cannot be empty so a new key will be assigned");
	}
	else if (bIsValidName)
	{
		// Valid name, so check it won't cause an identity conflict (only test if we have a single text selected to avoid confusion)
		const int32 NumTexts = EditableTextProperty->GetNumTexts();
		if (NumTexts == 1)
		{
			const FText PropertyValue = EditableTextProperty->GetText(0);

			const FString TextNamespace = FTextInspector::GetNamespace(PropertyValue).Get(FString());
			const FString TextKey = NewText.ToString();

			// Get the stable namespace and key that we should use for this property
			// If it comes back with the same namespace but a different key then it means there was an identity conflict
			FString NewNamespace;
			FString NewKey;
			const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
			EditableTextProperty->GetStableTextId(
				0,
				IEditableTextProperty::ETextPropertyEditAction::EditedKey,
				TextSource ? *TextSource : FString(),
				TextNamespace,
				TextKey,
				NewNamespace,
				NewKey
				);

			if (TextNamespace.Equals(NewNamespace, ESearchCase::CaseSensitive) && !TextKey.Equals(NewKey, ESearchCase::CaseSensitive))
			{
				ErrorMessage = LOCTEXT("TextKeyConflictErrorMsg", "Identity (namespace & key) is being used by a different text within this package so a new key will be assigned");
			}
		}
	}

	KeyEditableTextBox->SetError(ErrorMessage);
}

void SDlgTextPropertyEditableTextBox::OnKeyCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	if (!IsValidIdentity(NewText))
	{
		return;
	}

	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	// Don't commit the Multiple Values text if there are multiple properties being set
	if (NumTexts > 0 && (NumTexts == 1 || NewText.ToString() != MultipleValuesText.ToString()))
	{
		const FString& TextKey = NewText.ToString();
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Only apply the change if the new key is different - we want to keep the keys stable where possible
			const FString CurrentTextKey = FTextInspector::GetKey(PropertyValue).Get(FString());
			if (CurrentTextKey.Equals(TextKey, ESearchCase::CaseSensitive))
			{
				continue;
			}

			// Get the stable namespace and key that we should use for this property
			FString NewNamespace;
			FString NewKey;
			const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
			EditableTextProperty->GetStableTextId(
				TextIndex,
				IEditableTextProperty::ETextPropertyEditAction::EditedKey,
				TextSource ? *TextSource : FString(),
				FTextInspector::GetNamespace(PropertyValue).Get(FString()),
				TextKey,
				NewNamespace,
				NewKey
				);

			EditableTextProperty->SetText(TextIndex, FText::ChangeKey(NewNamespace, NewKey, PropertyValue));
		}
	}
}

FText SDlgTextPropertyEditableTextBox::GetPackageValue() const
{
	FText PackageValue;

	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		TOptional<FString> FoundNamespace = FTextInspector::GetNamespace(PropertyValue);
		if (FoundNamespace.IsSet())
		{
			PackageValue = FText::FromString(TextNamespaceUtil::ExtractPackageNamespace(FoundNamespace.GetValue()));
		}
	}
	else if (NumTexts > 1)
	{
		PackageValue = MultipleValuesText;
	}

	return PackageValue;
}

#endif // USE_STABLE_LOCALIZATION_KEYS

ECheckBoxState SDlgTextPropertyEditableTextBox::GetLocalizableCheckState(bool bActiveState) const
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();
	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);

		const bool bIsLocalized = !PropertyValue.IsCultureInvariant();
		return bIsLocalized == bActiveState ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	return ECheckBoxState::Undetermined;
}

void SDlgTextPropertyEditableTextBox::HandleLocalizableCheckStateChanged(ECheckBoxState InCheckboxState, bool bActiveState)
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	if (bActiveState)
	{
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Assign a key to any currently culture invariant texts
			if (PropertyValue.IsCultureInvariant())
			{
				// Get the stable namespace and key that we should use for this property
				FString NewNamespace;
				FString NewKey;
				EditableTextProperty->GetStableTextId(
					TextIndex,
					IEditableTextProperty::ETextPropertyEditAction::EditedKey,
					PropertyValue.ToString(),
					FString(),
					FString(),
					NewNamespace,
					NewKey
					);

				EditableTextProperty->SetText(TextIndex, FInternationalization::Get().ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(*PropertyValue.ToString(), *NewNamespace, *NewKey));
			}
		}
	}
	else
	{
		for (int32 TextIndex = 0; TextIndex < NumTexts; ++TextIndex)
		{
			const FText PropertyValue = EditableTextProperty->GetText(TextIndex);

			// Clear the identity from any non-culture invariant texts
			if (!PropertyValue.IsCultureInvariant())
			{
				const FString* TextSource = FTextInspector::GetSourceString(PropertyValue);
				EditableTextProperty->SetText(TextIndex, FText::AsCultureInvariant(PropertyValue.ToString()));
			}
		}
	}
}

EVisibility SDlgTextPropertyEditableTextBox::GetNamespaceOverridenWarningImageVisibility() const
{
	// Nothing to show
	if (GetLocalizableVisibility() != EVisibility::Visible)
	{
		return EVisibility::Collapsed;
	}

	return WillNamespaceBeUpdated() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SDlgTextPropertyEditableTextBox::GetTextWarningImageVisibility() const
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		return PropertyValue.IsCultureInvariant() ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return EVisibility::Collapsed;
}

bool SDlgTextPropertyEditableTextBox::IsValidIdentity(const FText& InIdentity, FText* OutReason, const FText* InErrorCtx) const
{
	const FString InvalidIdentityChars = FString::Printf(TEXT("%s%c%c"), INVALID_NAME_CHARACTERS, TextNamespaceUtil::PackageNamespaceStartMarker, TextNamespaceUtil::PackageNamespaceEndMarker);
	return FName::IsValidXName(InIdentity.ToString(), InvalidIdentityChars, OutReason, InErrorCtx);
}

#undef LOCTEXT_NAMESPACE
