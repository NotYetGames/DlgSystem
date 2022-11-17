// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "IPropertyTypeCustomization.h"
#include "Layout/Visibility.h"
#include "IDetailPropertyRow.h"

class FDlgTextPropertyPickList_CustomRowHelper;

class DLGSYSTEMEDITOR_API FDlgParticipantName_Details : public IPropertyTypeCustomization
{
	typedef FDlgParticipantName_Details Self;

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<Self>(); }

	void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
		FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
		IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	/** Gathers selectable options. */
	TArray<FName> GetAllParticipantNames() const;

private:
	// Cache the some property handles
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	// Cache the rows of the properties, created in CustomizeChildren
	TSharedPtr<FDlgTextPropertyPickList_CustomRowHelper> ParticipantNamePropertyRow;
};
