// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PropertyHandle.h"
#include "IPropertyUtilities.h"

#include "DlgSystem/DlgLocalizationHelper.h"
#include "DlgSystem/NYEngineVersionHelpers.h"

// FROM: FTextCustomization
// Allows us to edit a property handle
class DLGSYSTEMEDITOR_API FDlgEditableTextPropertyHandle : public IEditableTextProperty
{
public:
	FDlgEditableTextPropertyHandle(const TSharedRef<IPropertyHandle>& InPropertyHandle, const TSharedPtr<IPropertyUtilities>& InPropertyUtilities)
		: PropertyHandle(InPropertyHandle)
		, PropertyUtilities(InPropertyUtilities)
	{
	}

	bool IsMultiLineText() const override
	{
		return PropertyHandle->IsValidHandle() && PropertyHandle->GetMetaDataProperty()->GetBoolMetaData("MultiLine");
	}

	bool IsPassword() const override
	{
		return PropertyHandle->IsValidHandle() && PropertyHandle->GetMetaDataProperty()->GetBoolMetaData("PasswordField");
	}

	bool IsReadOnly() const override
	{
		return !PropertyHandle->IsValidHandle() || PropertyHandle->IsEditConst();
	}

	bool IsDefaultValue() const override
	{
		return PropertyHandle->IsValidHandle() && !PropertyHandle->DiffersFromDefault();
	}

	FText GetToolTipText() const override
	{
		return PropertyHandle->IsValidHandle() ? PropertyHandle->GetToolTipText() : FText::GetEmpty();
	}

	int32 GetNumTexts() const override
	{
		return (PropertyHandle->IsValidHandle())
			? PropertyHandle->GetNumPerObjectValues()
			: 0;
	}

	FText GetText(int32 InIndex) const override
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

	void SetText(int32 InIndex, const FText& InText) override
	{
		if (PropertyHandle->IsValidHandle())
		{
			FString ObjectValue;
			FTextStringHelper::WriteToBuffer(ObjectValue, InText);
			PropertyHandle->SetPerObjectValue(InIndex, ObjectValue);
		}
	}

	bool IsValidText(const FText& InText, FText& OutErrorMsg) const override
	{
		return true;
	}

#if USE_STABLE_LOCALIZATION_KEYS
	void GetStableTextId(
		int32 InIndex,
		ETextPropertyEditAction InEditAction,
		const FString& InTextSource,
		const FString& InProposedNamespace,
		const FString& InProposedKey,
		FString& OutStableNamespace, FString& OutStableKey
	) const override
	{
		if (PropertyHandle->IsValidHandle())
		{
			TArray<UPackage*> PropertyPackages;
			PropertyHandle->GetOuterPackages(PropertyPackages);

			check(PropertyPackages.IsValidIndex(InIndex));

			// NOTE: We use our copied version so that everything is in sync
			FDlgLocalizationHelper::StaticStableTextId(PropertyPackages[InIndex], InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
			//StaticStableTextId(PropertyPackages[InIndex], InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
		}
	}
#endif // USE_STABLE_LOCALIZATION_KEYS

#if NY_ENGINE_VERSION >= 501
	void RequestRefresh()
#else
	void RequestRefresh() override
#endif // NY_ENGINE_VERSION <= 500
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
