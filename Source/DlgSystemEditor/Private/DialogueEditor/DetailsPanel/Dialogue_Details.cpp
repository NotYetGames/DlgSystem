// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "Dialogue_Details.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#include "DlgDialogue.h"

#define LOCTEXT_NAMESPACE "Dialogue_Details"

void FDialogue_Details::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
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
	// GeneratedCategory.AddProperty(UDlgDialogue::GetMemberNameDlgName(), DialogueClass);
	// GeneratedCategory.AddProperty(UDlgDialogue::GetMemberNameDlgGuid(), DialogueClass);
	// GeneratedCategory.AddProperty(UDlgDialogue::GetMemberNameDlgData(), DialogueClass);

	IDetailCategoryBuilder& DataCategory = DetailBuilder.EditCategory(TEXT("DialogueData"));
	DataCategory.InitiallyCollapsed(false);
	// DataCategory.AddProperty(UDlgDialogue::GetMemberNameStartNode(), DialogueClass)
	// 	.ShouldAutoExpand(true);
	// DataCategory.AddProperty(UDlgDialogue::GetMemberNameNodes(), DialogueClass)
	// 	.ShouldAutoExpand(true);
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
