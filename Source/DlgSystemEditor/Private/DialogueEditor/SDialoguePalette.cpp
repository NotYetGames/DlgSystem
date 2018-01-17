// Fill out your copyright notice in the Description page of Project Settings.
#include "SDialoguePalette.h"

#include "DialogueEditor/Graph/DialogueGraphSchema.h"

void SDialoguePalette::Construct(const FArguments& InArgs)
{
	// Auto expand the palette as there's so few nodes
	SGraphPalette::Construct(SGraphPalette::FArguments().AutoExpandActionMenu(true));
}

void SDialoguePalette::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	const UDialogueGraphSchema* Schema = GetDefault<UDialogueGraphSchema>();
	FGraphActionMenuBuilder ActionMenuBuilder;

	// Determine all possible actions
	Schema->GetPaletteActions(ActionMenuBuilder);
	OutAllActions.Append(ActionMenuBuilder);
}
