// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphPalette.h"
#include "GraphEditor.h"
#include "SGraphActionMenu.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "DlgSystem/NYEngineVersionHelpers.h"

//////////////////////////////////////////////////////////////////////////
/**
 * Widget that appears after right clicking on the graph
 */
class DLGSYSTEMEDITOR_API SDlgDialogueActionMenu : public SBorder
{
	typedef SDlgDialogueActionMenu Self;
public:
	/** Delegate for the OnCloseReason event which is always raised when the SDlgDialogueActionMenu closes */
	DECLARE_DELEGATE_TwoParams(FDialogueActionMenuClosedReason, bool /*bActionExecuted*/, bool /*bGraphPinContext*/);

	SLATE_BEGIN_ARGS(Self)
		: _Graph(nullptr)
		, _NewNodePosition(FNYVector2f::ZeroVector)
		, _AutoExpandActionMenu(true)
	{}

		SLATE_ARGUMENT(UEdGraph*, Graph)
		SLATE_ARGUMENT(FNYVector2f, NewNodePosition)
		SLATE_ARGUMENT(TArray<UEdGraphPin*>, DraggedFromPins)
		SLATE_ARGUMENT(SGraphEditor::FActionMenuClosed, OnClosedCallback)
		SLATE_ARGUMENT(bool, AutoExpandActionMenu)
		SLATE_EVENT(FDialogueActionMenuClosedReason, OnCloseReason)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SDlgDialogueActionMenu();

	TSharedPtr<SEditableTextBox> GetFilterTextBox() const { return GraphActionMenu->GetFilterTextBox(); }

protected:
	void OnActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& SelectedAction, ESelectInfo::Type InSelectionType);

	/** Callback used to populate all actions list in SGraphActionMenu */
	void CollectAllActions(FGraphActionListBuilderBase& OutAllActions);

protected:
	UEdGraph* Graph = nullptr;

	/** The pins this node was dragged from. */
	TArray<UEdGraphPin*> DraggedFromPins;

	/** The position of this new node */
	FNYVector2f NewNodePosition;

	/** Should expand the new menu? */
	bool AutoExpandActionMenu = true;

	/** Was an action executed? */
	bool bActionExecuted = false;

	// Events for close
	SGraphEditor::FActionMenuClosed OnClosedCallback;
	FDialogueActionMenuClosedReason OnCloseReasonCallback;

	// The actual menu
	TSharedPtr<SGraphActionMenu> GraphActionMenu;
};
