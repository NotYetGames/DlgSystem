// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgEditorPalette.h"

#include "DlgSystemEditor/Editor/Graph/DialogueGraphSchema.h"

void SDlgEditorPalette::Construct(const FArguments& InArgs)
{
	// Auto expand the palette as there's so few nodes
	SGraphPalette::Construct(SGraphPalette::FArguments().AutoExpandActionMenu(true));
}

void SDlgEditorPalette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	const UDialogueGraphSchema* Schema = GetDefault<UDialogueGraphSchema>();
	FGraphActionMenuBuilder ActionMenuBuilder;

	// Determine all possible actions
	Schema->GetPaletteActions(ActionMenuBuilder);
	OutAllActions.Append(ActionMenuBuilder);
}
