// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueEdge_Details.h"

#include "IDetailPropertyRow.h"
#include "PropertyEditing.h"

#include "DlgNode.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "CustomRowHelpers/MultiLineEditableTextBox_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueEdge_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueEdge_Details
void FDialogueEdge_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = DetailsPanel::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	bShowTextProperty = true;

	// Should we show hide the Text property?
	if (UDialogueGraphNode* GraphNode = DetailsPanel::GetClosestGraphNodeFromPropertyHandle(StructPropertyHandle.ToSharedRef()))
	{
		// Virtual parents do not handle direct children, only grand children
		// And selector node do not even touch them
		bShowTextProperty = !GraphNode->IsVirtualParentNode() && !GraphNode->IsSelectorNode();
	}

	const bool bShowOnlyInnerProperties = StructPropertyHandle->GetProperty()->HasMetaData(META_ShowOnlyInnerProperties);
	if (!bShowOnlyInnerProperties)
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			];
	}
}

void FDialogueEdge_Details::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, TargetIndex)).ToSharedRef());
	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, Conditions)).ToSharedRef());

	// Text
	TextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, Text));
	FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

	TextPropertyRow = MakeShareable(new FMultiLineEditableTextBox_CustomRowHelper(DetailWidgetRow, TextPropertyHandle));
	TextPropertyRow->SetMultiLineEditableTextBoxWidget(
		SNew(SMultiLineEditableTextBox)
		.Font(FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
		.SelectAllTextWhenFocused(false)
		.ClearKeyboardFocusOnCommit(false)
		.SelectAllTextOnCommit(false)
		.AutoWrapText(true)
		.ModiferKeyForNewLine(DetailsPanel::GetModifierKeyFromDialogueSettings())
		.Text(TextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::GetTextValue)
		.OnTextCommitted(TextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::HandleTextCommited)
	)
	->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetTextVisibility))
	->Update();
}

#undef LOCTEXT_NAMESPACE
