// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueEdge_Details.h"

#include "IDetailPropertyRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyTypeCustomization.h"

#include "Nodes/DlgNode.h"
#include "DialogueDetailsPanelUtils.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "CustomRowHelpers/MultiLineEditableTextBox_CustomRowHelper.h"
#include "STextPropertyPickList.h"
#include "CustomRowHelpers/TextPropertyPickList_CustomRowHelper.h"

#define LOCTEXT_NAMESPACE "DialogueEdge_Details"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueEdge_Details
void FDialogueEdge_Details::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	Dialogue = FDialogueDetailsPanelUtils::GetDialogueFromPropertyHandle(StructPropertyHandle.ToSharedRef());
	bShowTextProperty = true;

	// Should we show hide the Text property?
	if (const UDialogueGraphNode* GraphNode = FDialogueDetailsPanelUtils::GetClosestGraphNodeFromPropertyHandle(StructPropertyHandle.ToSharedRef()))
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

	// Speaker State
	{
		const TSharedPtr<IPropertyHandle> SpeakerStatePropertyHandle =
			StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, SpeakerState));

		FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("SpeakerStateSearchKey", "Speaker State"));

		SpeakerStatePropertyRow = MakeShared<FTextPropertyPickList_CustomRowHelper>(DetailWidgetRow, SpeakerStatePropertyHandle);
		SpeakerStatePropertyRow->SetTextPropertyPickListWidget(
			SNew(STextPropertyPickList)
			.AvailableSuggestions(this, &Self::GetAllDialoguesSpeakerStates)
			.OnTextCommitted(this, &Self::HandleSpeakerStateCommitted)
			.HasContextCheckbox(false)
		)
		->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetSpeakerStateVisibility))
		->Update();
	}

	// Text
	TextPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, Text));
	FDetailWidgetRow* DetailWidgetRow = &StructBuilder.AddCustomRow(LOCTEXT("TextSearchKey", "Text"));

	TextPropertyRow = MakeShared<FMultiLineEditableTextBox_CustomRowHelper>(DetailWidgetRow, TextPropertyHandle);
	TextPropertyRow->SetMultiLineEditableTextBoxWidget(
		SNew(SMultiLineEditableTextBox)
		.Font(FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
		.SelectAllTextWhenFocused(false)
		.ClearKeyboardFocusOnCommit(false)
		.SelectAllTextOnCommit(false)
		.AutoWrapText(true)
		.ModiferKeyForNewLine(FDialogueDetailsPanelUtils::GetModifierKeyFromDialogueSettings())
		.Text(TextPropertyRow.ToSharedRef(), &FMultiLineEditableTextBox_CustomRowHelper::GetTextValue)
		.OnTextCommitted(this, &Self::HandleTextCommitted)
	)
	->SetPropertyUtils(StructCustomizationUtils.GetPropertyUtilities())
	->SetVisibility(CREATE_VISIBILITY_CALLBACK(&Self::GetTextVisibility))
	->Update();

	StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDlgEdge, TextArguments)).ToSharedRef())
		.Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetTextVisibility));

	IDetailPropertyRow& BoolPropertyRow = StructBuilder.AddProperty(StructPropertyHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FDlgEdge, bIncludeInAllOptionListIfUnsatisfied)).ToSharedRef()
	);
	BoolPropertyRow.Visibility(CREATE_VISIBILITY_CALLBACK(&Self::GetTextVisibility));
}

void FDialogueEdge_Details::HandleSpeakerStateCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo)
{
	if (Dialogue)
	{
		Dialogue->RefreshData();
	}
}

void FDialogueEdge_Details::HandleTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
{
	TextPropertyRow->HandleTextCommited(InText, CommitInfo);

	if (UDialogueGraphNode_Edge* GraphEdge = FDialogueDetailsPanelUtils::GetAsGraphNodeEdgeFromPropertyHandle(StructPropertyHandle.ToSharedRef()))
	{
		GraphEdge->GetDialogueEdge().RebuildTextArgumentsArray();

		if (Dialogue)
		{
			Dialogue->RefreshData();
		}
	}
}

#undef LOCTEXT_NAMESPACE
