// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgNode_Details.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Text/STextBlock.h"
#include "DetailWidgetRow.h"

#include "DlgSystem/Nodes/DlgNode.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"

#define LOCTEXT_NAMESPACE "DialogueNode_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgNode_Details
void FDlgNode_Details::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	// Only support one object being customized
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	// NOTE currently not used, see FDialogueGraphNodeDetails
	IDetailCategoryBuilder& DataCategory = DetailBuilder.EditCategory(TEXT("Do we need this?"));
	DataCategory.InitiallyCollapsed(false);
	DataCategory.AddCustomRow(NSLOCTEXT("TEST", "MyWarningRowFilterString", "Search Filter Keywords"))
		.Visibility(EVisibility::Visible)
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MyWarningTest", "BaseString should not be empty!"))
		];
}

#undef LOCTEXT_NAMESPACE
