// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "MultiLineEditableTextBox_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Input/SCheckBox.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "IPropertyUtilities.h"

#define LOCTEXT_NAMESPACE "MultiLineEditableTextBox_CustomRowHelper"

const FText FMultiLineEditableTextBox_CustomRowHelper::MultipleValuesText = NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values");

namespace
{
	/** Allows us to edit a property handle */
	class FDlgEditableTextPropertyHandle : public IEditableTextProperty
	{
	public:
		FDlgEditableTextPropertyHandle(const TSharedRef<IPropertyHandle>& InPropertyHandle, const TSharedPtr<IPropertyUtilities>& InPropertyUtilities)
			: PropertyHandle(InPropertyHandle)
			, PropertyUtilities(InPropertyUtilities)
		{
		}

		virtual bool IsMultiLineText() const override
		{
			return PropertyHandle->IsValidHandle() && PropertyHandle->GetMetaDataProperty()->GetBoolMetaData("MultiLine");
		}

		virtual bool IsPassword() const override
		{
			return PropertyHandle->IsValidHandle() && PropertyHandle->GetMetaDataProperty()->GetBoolMetaData("PasswordField");
		}

		virtual bool IsReadOnly() const override
		{
			return !PropertyHandle->IsValidHandle() || PropertyHandle->IsEditConst();
		}

		virtual bool IsDefaultValue() const override
		{
			return PropertyHandle->IsValidHandle() && !PropertyHandle->DiffersFromDefault();
		}

		virtual FText GetToolTipText() const override
		{
			return (PropertyHandle->IsValidHandle())
				? PropertyHandle->GetToolTipText()
				: FText::GetEmpty();
		}

		virtual int32 GetNumTexts() const override
		{
			return (PropertyHandle->IsValidHandle())
				? PropertyHandle->GetNumPerObjectValues()
				: 0;
		}

		virtual FText GetText(const int32 InIndex) const override
		{
			if (PropertyHandle->IsValidHandle())
			{
				FString ObjectValue;
				if (PropertyHandle->GetPerObjectValue(InIndex, ObjectValue) == FPropertyAccess::Success)
				{
					FText TextValue;
					if (FTextStringHelper::ReadFromBuffer(*ObjectValue, TextValue))
					{
						return TextValue;
					}
				}
			}

			return FText::GetEmpty();
		}

		virtual void SetText(const int32 InIndex, const FText& InText) override
		{
			if (PropertyHandle->IsValidHandle())
			{
				FString ObjectValue;
				FTextStringHelper::WriteToBuffer(ObjectValue, InText);
				PropertyHandle->SetPerObjectValue(InIndex, ObjectValue);
			}
		}

		virtual bool IsValidText(const FText& InText, FText& OutErrorMsg) const override
		{
			return true;
		}

#if USE_STABLE_LOCALIZATION_KEYS
		virtual void GetStableTextId(const int32 InIndex, const ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey) const override
		{
			if (PropertyHandle->IsValidHandle())
			{
				TArray<UPackage*> PropertyPackages;
				PropertyHandle->GetOuterPackages(PropertyPackages);

				check(PropertyPackages.IsValidIndex(InIndex));

				StaticStableTextId(PropertyPackages[InIndex], InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
			}
		}
#endif // USE_STABLE_LOCALIZATION_KEYS

		virtual void RequestRefresh() override
		{
			if (PropertyUtilities.IsValid())
			{
				PropertyUtilities->RequestRefresh();
			}
		}

	private:
		TSharedRef<IPropertyHandle> PropertyHandle;
		TSharedPtr<IPropertyUtilities> PropertyUtilities;
	};
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FMultiLineEditableTextBox_CustomRowHelper
FText FMultiLineEditableTextBox_CustomRowHelper::GetResetToolTip() const
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

EVisibility FMultiLineEditableTextBox_CustomRowHelper::GetDiffersFromDefaultAsVisibility() const
{
	return EVisibility::Visible;
}

FReply FMultiLineEditableTextBox_CustomRowHelper::OnResetClicked()
{
	if (PropertyHandle.IsValid())
	{
		PropertyHandle->ResetToDefault();
	}
	return FReply::Handled();
}

bool FMultiLineEditableTextBox_CustomRowHelper::CanEdit() const
{
	const bool bIsReadOnly = FTextLocalizationManager::Get().IsLocalizationLocked() || EditableTextProperty->IsReadOnly();
	return !bIsReadOnly;
}

bool FMultiLineEditableTextBox_CustomRowHelper::IsCultureInvariantFlagEnabled() const
{
	return !IsSourceTextReadOnly();
}

bool FMultiLineEditableTextBox_CustomRowHelper::IsSourceTextReadOnly() const
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

bool FMultiLineEditableTextBox_CustomRowHelper::IsIdentityReadOnly() const
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

FText FMultiLineEditableTextBox_CustomRowHelper::GetNamespaceValue() const
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

void FMultiLineEditableTextBox_CustomRowHelper::OnNamespaceChanged(const FText& NewText)
{
	FText ErrorMessage;
	const FText ErrorCtx = LOCTEXT("TextNamespaceErrorCtx", "Namespace");
	IsValidIdentity(NewText, &ErrorMessage, &ErrorCtx);
	NamespaceEditableTextBox->SetError(ErrorMessage);
}

void FMultiLineEditableTextBox_CustomRowHelper::OnNamespaceCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
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

FText FMultiLineEditableTextBox_CustomRowHelper::GetKeyValue() const
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
void FMultiLineEditableTextBox_CustomRowHelper::OnKeyChanged(const FText& NewText)
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

void FMultiLineEditableTextBox_CustomRowHelper::OnKeyCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
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

FText FMultiLineEditableTextBox_CustomRowHelper::GetPackageValue() const
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

ECheckBoxState FMultiLineEditableTextBox_CustomRowHelper::GetLocalizableCheckState(bool bActiveState) const
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

void FMultiLineEditableTextBox_CustomRowHelper::HandleLocalizableCheckStateChanged(ECheckBoxState InCheckboxState, bool bActiveState)
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

bool FMultiLineEditableTextBox_CustomRowHelper::IsValidIdentity(const FText& InIdentity, FText* OutReason, const FText* InErrorCtx) const
{
	const FString InvalidIdentityChars = FString::Printf(TEXT("%s%c%c"), INVALID_NAME_CHARACTERS, TextNamespaceUtil::PackageNamespaceStartMarker, TextNamespaceUtil::PackageNamespaceEndMarker);
	return FName::IsValidXName(InIdentity.ToString(), InvalidIdentityChars, OutReason, InErrorCtx);
}

EVisibility FMultiLineEditableTextBox_CustomRowHelper::GetTextWarningImageVisibility() const
{
	const int32 NumTexts = EditableTextProperty->GetNumTexts();

	if (NumTexts == 1)
	{
		const FText PropertyValue = EditableTextProperty->GetText(0);
		return PropertyValue.IsCultureInvariant() ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return EVisibility::Collapsed;
}

void FMultiLineEditableTextBox_CustomRowHelper::UpdateInternal()
{
	check(MultiLineEditableTextBoxWidget.IsValid());
	check(PropertyHandle.IsValid());
	check(PropertyUtils.IsValid());
	MultiLineEditableTextBoxWidget->SetToolTipText(ToolTip);

	PropertyHandle->MarkResetToDefaultCustomized(true);
	EditableTextProperty = MakeShared<FDlgEditableTextPropertyHandle>(PropertyHandle.ToSharedRef(), PropertyUtils);

	TSharedPtr<SHorizontalBox> HorizontalBox;
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
		SAssignNew(HorizontalBox, SHorizontalBox)
		+SHorizontalBox::Slot()
		.Padding(0.f, 0.f, 4.f, 0.f)
		.FillWidth(1.f)
		[
			MultiLineEditableTextBoxWidget.ToSharedRef()
		]
	];

	// From STextPropertyEditableTextBox
	// Add Edit advanced text settings
	HorizontalBox->AddSlot()
	.AutoWidth()
	[
		SNew(SComboButton)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.ContentPadding(FMargin(4, 0))
		.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
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
					.TextStyle(FEditorStyle::Get(), "LargeText")
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
							.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
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
							.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
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
				]
				+SGridPanel::Slot(1, 2)
				.Padding(2)
				[
					SNew(SEditableTextBox)
					.Text(this, &Self::GetPackageValue)
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
					.IsReadOnly(this, &Self::IsIdentityReadOnly)
				]

				// Key
				+SGridPanel::Slot(0, 4)
				.Padding(2)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TextKeyLabel", "Key:"))
				]
				+SGridPanel::Slot(1, 4)
				.Padding(2)
				[
					SAssignNew(KeyEditableTextBox, SEditableTextBox)
					.Text(this, &Self::GetKeyValue)
#if USE_STABLE_LOCALIZATION_KEYS
					.SelectAllTextWhenFocused(true)
					.ClearKeyboardFocusOnCommit(false)
					.OnTextChanged(this, &Self::OnKeyChanged)
					.OnTextCommitted(this, &Self::OnKeyCommitted)
					.SelectAllTextOnCommit(true)
					.IsReadOnly(this, &Self::IsIdentityReadOnly)
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
					.TextStyle(FEditorStyle::Get(), "LargeText")
					.Text(LOCTEXT("TextReferencedTextLabel", "Referenced Text"))
				]

				// String Table
				+SGridPanel::Slot(0, 6)
				.Padding(2)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TextStringTableLabel", "String Table:"))
				]
				+SGridPanel::Slot(1, 6)
				.Padding(2)
				[
					SNew(STextPropertyEditableStringTableReference, EditableTextProperty.ToSharedRef())
					.AllowUnlink(true)
					.IsEnabled(this, &Self::CanEdit)
				]
			]
		]
	];

	// Add warning
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

void FMultiLineEditableTextBox_CustomRowHelper::HandleTextCommited(const FText& NewText, ETextCommit::Type CommitInfo)
{
	FText CurrentText;
	if (PropertyHandle->GetValueAsFormattedText(CurrentText) != FPropertyAccess::MultipleValues ||
		NewText.ToString() != MultipleValuesText.ToString())
	{
		PropertyHandle->SetValueFromFormattedString(NewText.ToString());
	}
}

FText FMultiLineEditableTextBox_CustomRowHelper::GetTextValue() const
{
	FText Text;

	// Sets the value
	if (PropertyHandle->GetValueAsFormattedText(Text) == FPropertyAccess::MultipleValues)
	{
		// If multiple values
		Text = MultipleValuesText;
	}

	return Text;
}

#undef LOCTEXT_NAMESPACE
