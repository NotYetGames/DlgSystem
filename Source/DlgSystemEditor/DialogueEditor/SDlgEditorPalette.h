// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphPalette.h"

//////////////////////////////////////////////////////////////////////////

class DLGSYSTEMEDITOR_API SDlgEditorPalette : public SGraphPalette
{
public:
	SLATE_BEGIN_ARGS(SDlgEditorPalette) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	/** Callback used to populate all actions list in SGraphActionMenu */
	void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override;
};
