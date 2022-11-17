// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "KismetNodes/SGraphNodeK2Base.h"

class UDialogueK2Node_Select;

class SGraphNode_DialogueK2Select : public SGraphNodeK2Base
{
	typedef SGraphNodeK2Base Super;
	typedef SGraphNode_DialogueK2Select Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogueK2Node_Select* InNode);

	// SGraphNode interface
	void CreatePinWidgets() override;

protected:
	void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	EVisibility IsAddPinButtonVisible() const override;
	FReply OnAddPin() override;
	// End of SGraphNode interface

private:
	// The blueprint dialogue select this view represents
	UDialogueK2Node_Select* DialogueK2Node_Select = nullptr;
};
