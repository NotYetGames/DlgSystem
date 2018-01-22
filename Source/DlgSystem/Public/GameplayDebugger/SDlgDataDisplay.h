// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SCompoundWidget.h"

/**
 * Implements the Runtime Dialogue Data Display
 */
class SDlgDataDisplay : public SCompoundWidget
{
	typedef SDlgDataDisplay Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
