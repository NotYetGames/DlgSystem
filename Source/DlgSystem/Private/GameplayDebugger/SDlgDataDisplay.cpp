// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "SDlgDataDisplay.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataDisplay
void SDlgDataDisplay::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor::Gray) // Darken the outer border
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("TODO")))
			]
		]
	];
}
