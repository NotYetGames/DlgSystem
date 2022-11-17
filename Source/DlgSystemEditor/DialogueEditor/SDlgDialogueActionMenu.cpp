// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgDialogueActionMenu.h"

#include "EdGraph/EdGraph.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"

#include "DlgSystem/NYEngineVersionHelpers.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDialogueActionMenu
SDlgDialogueActionMenu::~SDlgDialogueActionMenu()
{
	OnClosedCallback.ExecuteIfBound();
	OnCloseReasonCallback.ExecuteIfBound(bActionExecuted, DraggedFromPins.Num() > 0);
}

void SDlgDialogueActionMenu::Construct(const FArguments& InArgs)
{
	bActionExecuted = false;
	Graph = InArgs._Graph;
	DraggedFromPins = InArgs._DraggedFromPins;
	NewNodePosition = InArgs._NewNodePosition;
	OnClosedCallback = InArgs._OnClosedCallback;
	AutoExpandActionMenu = InArgs._AutoExpandActionMenu;
	OnCloseReasonCallback = InArgs._OnCloseReason;

	// Build the widget layout
	SBorder::Construct(SBorder::FArguments()
		.BorderImage(FNYAppStyle::GetBrush("Menu.Background"))
		.Padding(5)
		[
			// Achieving fixed width by nesting items within a fixed width box.
			SNew(SBox)
			.WidthOverride(400)
			.HeightOverride(400)
			[
				SAssignNew(GraphActionMenu, SGraphActionMenu)
				.OnActionSelected(this, &Self::OnActionSelected)
				.OnCollectAllActions(this, &Self::CollectAllActions)
				.AutoExpandActionMenu(AutoExpandActionMenu)
			]
		]
	);
}

void SDlgDialogueActionMenu::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	// Build up the context object
	FGraphContextMenuBuilder ContextMenuBuilder(Graph);
	if (DraggedFromPins.Num() > 0)
	{
		ContextMenuBuilder.FromPin = DraggedFromPins[0];
	}

	// Determine all possible actions
	Graph->GetSchema()->GetGraphContextActions(ContextMenuBuilder);

	// Copy the added options back to the main list
	OutAllActions.Append(ContextMenuBuilder);
}

void SDlgDialogueActionMenu::OnActionSelected(const TArray<TSharedPtr<FEdGraphSchemaAction>>& SelectedAction, ESelectInfo::Type InSelectionType)
{
	if (!IsValid(Graph))
	{
		return;
	}

	if (InSelectionType == ESelectInfo::OnMouseClick || InSelectionType == ESelectInfo::OnKeyPress || SelectedAction.Num() == 0)
	{
		for (int32 ActionIndex = 0; ActionIndex < SelectedAction.Num(); ActionIndex++)
		{
			TSharedPtr<FEdGraphSchemaAction> CurrentAction = SelectedAction[ActionIndex];

			if (CurrentAction.IsValid())
			{
				if (!bActionExecuted)
				{
					FSlateApplication::Get().DismissAllMenus();
					bActionExecuted = true;
				}

				CurrentAction->PerformAction(Graph, DraggedFromPins, NewNodePosition);
			}
		}
	}
}
