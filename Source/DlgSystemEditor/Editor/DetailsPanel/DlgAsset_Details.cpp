// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgAsset_Details.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#include "DlgSystem/DlgDialogue.h"

#define LOCTEXT_NAMESPACE "Dialogue_Details"

void FDlgAsset_Details::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	// Only support one object being customized
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	UClass* DialogueClass = UDlgDialogue::StaticClass();
	// IDetailCategoryBuilder& GeneratedCategory = DetailBuilder.EditCategory(TEXT("Generated"));
	// GeneratedCategory.InitiallyCollapsed(false);
	// GeneratedCategory.AddProperty(UDlgDialogue::GetMemberNameName(), DialogueClass);
	// GeneratedCategory.AddProperty(UDlgDialogue::GetMemberNameGUID(), DialogueClass);
	// GeneratedCategory.AddProperty(UDlgDialogue::GetMemberNameParticipantsData(), DialogueClass);

	IDetailCategoryBuilder& DataCategory = DetailBuilder.EditCategory(TEXT("DialogueData"));
	DataCategory.InitiallyCollapsed(false);
	// DataCategory.AddProperty(UDlgDialogue::GetMemberNameStartNode(), DialogueClass)
	// 	.ShouldAutoExpand(true);
	// DataCategory.AddProperty(UDlgDialogue::GetMemberNameNodes(), DialogueClass)
	// 	.ShouldAutoExpand(true);
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
