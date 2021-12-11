// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueParticipantName_Details.h"

#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "Components/PrimitiveComponent.h"

#include "DialogueEditor/DetailsPanel/Widgets/SDialogueTextPropertyPickList.h"
#include "DialogueEditor/DetailsPanel/Widgets/DialogueTextPropertyPickList_CustomRowHelper.h"

#include "DlgParticipantName.h"
#include "DlgManager.h"

#define LOCTEXT_NAMESPACE "DialogueParticipantName_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FNYWhistlerComponentNameCustomization
void FDialogueParticipantName_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
}

void FDialogueParticipantName_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// ParticipantName
	{
		const TSharedPtr<IPropertyHandle> ParticipantNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgParticipantName, ParticipantName));
		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(InStructPropertyHandle->GetPropertyDisplayName());
		ParticipantNamePropertyRow = MakeShared<FDialogueTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, ParticipantNamePropertyHandle);
		ParticipantNamePropertyRow->SetTextPropertyPickListWidget(
			SNew(SDialogueTextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllParticipantNames)
			.HasContextCheckbox(false)
		);
		ParticipantNamePropertyRow->SetParentStructPropertyHandle(InStructPropertyHandle);
		ParticipantNamePropertyRow->Update();
	}
}


TArray<FName> FDialogueParticipantName_Details::GetAllParticipantNames() const
{
	return UDlgManager::GetDialoguesParticipantNames();
}

#undef LOCTEXT_NAMESPACE


