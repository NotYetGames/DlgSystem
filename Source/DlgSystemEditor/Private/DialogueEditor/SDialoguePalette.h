// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphPalette.h"

//////////////////////////////////////////////////////////////////////////

class SDialoguePalette : public SGraphPalette
{
public:
	SLATE_BEGIN_ARGS(SDialoguePalette) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	/** Callback used to populate all actions list in SGraphActionMenu */
	void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override;
};
