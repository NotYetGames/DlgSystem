// Copyright 2017-2018 Csaba Molnar, Daniel Butum
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
