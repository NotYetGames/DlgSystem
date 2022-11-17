// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgParticipantName_Details.h"

#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"

#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/SDlgTextPropertyPickList.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgTextPropertyPickList_CustomRowHelper.h"

#include "DlgSystem/DlgParticipantName.h"
#include "DlgSystem/DlgManager.h"

#define LOCTEXT_NAMESPACE "DialogueParticipantName_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FNYWhistlerComponentNameCustomization
void FDlgParticipantName_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
}

void FDlgParticipantName_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// ParticipantName
	{
		const TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgParticipantName, ParticipantName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(InStructPropertyHandle->GetPropertyDisplayName());
		ParticipantNamePropertyRow = MakeShared<FDlgTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDlgTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllParticipantNames)
			.HasContextCheckbox(false)
		);
		ParticipantNamePropertyRow->SetParentStructPropertyHandle(InStructPropertyHandle);
		ParticipantNamePropertyRow->Update();
	}
}


TArray<FName> FDlgParticipantName_Details::GetAllParticipantNames() const
{
	return UDlgManager::GetDialoguesParticipantNames();
}

#undef LOCTEXT_NAMESPACE
