// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgDataDisplayTreeNode.h"

#define LOCTEXT_NAMESPACE "FDlgDataDisplayTreeNode"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgDataDisplayTreeNode
FDlgDataDisplayTreeNode::FDlgDataDisplayTreeNode(const FText& InDisplayText, TSharedPtr<Self> InParent)
	: DisplayText(InDisplayText), Parent(InParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueBrowserTreeRootNode
FDlgDataDisplayTreeRootNode::FDlgDataDisplayTreeRootNode() :
	Super(FText::FromString(TEXT("ROOT")), nullptr)
{
}

#undef LOCTEXT_NAMESPACE
