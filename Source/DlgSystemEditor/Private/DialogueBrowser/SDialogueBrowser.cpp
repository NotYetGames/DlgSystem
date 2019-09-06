// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "SDialogueBrowser.h"

#include "Editor.h"
#include "Toolkits/AssetEditorManager.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SMissingWidget.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"

#include "DlgManager.h"
#include "DlgDialogue.h"
#include "DialogueStyle.h"
#include "DialogueSearch/DialogueSearchUtilities.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueBrowserUtilities.h"
#include "TreeViewHelpers/DlgTreeViewHelper.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "SDialogueBrowser"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename ItemType>
class SCategoryHeaderTableRow : public STableRow<ItemType>
{
private:
	typedef SCategoryHeaderTableRow Self;

public:
	SLATE_BEGIN_ARGS(SCategoryHeaderTableRow)
	{}
		SLATE_DEFAULT_SLOT(typename SCategoryHeaderTableRow::FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		STableRow<ItemType>::ChildSlot
		.Padding(8.0f, 2.0f, 0.0f, 0.0f)
		[
			SAssignNew(ContentBorder, SBorder)
			.BorderImage(this, &Self::GetBackgroundImage)
			.Padding(FMargin(1.0f, 3.0f))
			.BorderBackgroundColor(FLinearColor(.6, .6, .6, 1.0f))
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(2.0f, 2.0f, 2.0f, 2.0f)
				.AutoWidth()
				[
					SNew(SExpanderArrow, STableRow<ItemType>::SharedThis(this))
				]

				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					InArgs._Content.Widget
				]
			]
		];

		STableRow<ItemType>::ConstructInternal(
			typename STableRow<ItemType>::FArguments()
			.Style(FEditorStyle::Get(), "DetailsView.TreeView.TableRow")
			.ShowSelection(false),
			InOwnerTableView
		);
	}

	const FSlateBrush* GetBackgroundImage() const
	{
		if (STableRow<ItemType>::IsHovered())
		{
			return STableRow<ItemType>::IsItemExpanded() ? FEditorStyle::GetBrush("DetailsView.CategoryTop_Hovered") : FEditorStyle::GetBrush("DetailsView.CollapsedCategory_Hovered");
		}

		return STableRow<ItemType>::IsItemExpanded() ? FEditorStyle::GetBrush("DetailsView.CategoryTop") : FEditorStyle::GetBrush("DetailsView.CollapsedCategory");
	}

	void SetContent(TSharedRef<SWidget> InContent) override
	{
		ContentBorder->SetContent(InContent);
	}

	void SetRowContent(TSharedRef< SWidget > InContent) override
	{
		ContentBorder->SetContent(InContent);
	}

private:
	TSharedPtr<SBorder> ContentBorder;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDialogueBrowser
void SDialogueBrowser::Construct(const FArguments& InArgs)
{
	DefaultSortOption = MakeShared<FDialogueBrowserSortOption>(EDialogueBrowserSortOption::DialogueReferences, TEXT("Dialogue References"));
	SelectedSortOption = DefaultSortOption;
	SortOptions = {
		DefaultSortOption,
		MakeShared<FDialogueBrowserSortOption>(EDialogueBrowserSortOption::Name, TEXT("Name"))
	};

	RootTreeItem = MakeShared<FDialogueBrowserTreeRootNode>();
	ParticipantsTreeView = SNew(STreeView<TSharedPtr<FDialogueBrowserTreeNode>>)
		.ItemHeight(32)
		.TreeItemsSource(&RootChildren)
		.OnGenerateRow(this, &Self::HandleGenerateRow)
		.OnSelectionChanged(this, &Self::HandleTreeSelectionChanged)
		.OnGetChildren(this, &Self::HandleGetChildren)
		.SelectionMode(ESelectionMode::Single)
		.OnMouseButtonDoubleClick(this, &Self::HandleDoubleClick)
		.OnSetExpansionRecursive(this, &Self::HandleSetExpansionRecursive)
		.HeaderRow(
			SNew(SHeaderRow)
			.Visibility(EVisibility::Visible)
			+SHeaderRow::Column(NAME_Name)
			.DefaultLabel(LOCTEXT("Name", "Participant Name"))
		);

	ChildSlot
	[
		SNew(SBorder)
		.Padding(FMargin(3.f))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Top bar
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f, 1.f)
			[
				SNew(SHorizontalBox)

				// Search Input
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.f, 0.f, 4.f, 0.f)
				[
					GetFilterTextBoxWidget()
				]

				// View Options
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 2.0f)
				[
					SNew(SComboButton)
					.ComboButtonStyle(FEditorStyle::Get(), "GenericFilters.ComboButtonStyle")
					.ForegroundColor(FLinearColor::White)
					.ContentPadding(0)
					.ToolTipText(LOCTEXT("View_Tooltip", "View Options for the Dialogue Browser"))
					.OnGetMenuContent(this, &Self::FillViewOptionsEntries)
					.HasDownArrow(true)
					.ContentPadding(FMargin(1, 0))
					.ButtonContent()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
							.Text(FText::FromString(FString(TEXT("\xf0b0"))) /*fa-filter*/)
						]
						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
							.Text(LOCTEXT("View_Key", "View Options"))
						]
					]
				]

				// Sort menu
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SComboBox<SortOptionType>)
					.OptionsSource(&SortOptions)
					.InitiallySelectedItem(DefaultSortOption)
					.ContentPadding(FMargin(4.0, 2.0))
					.OnGenerateWidget_Lambda([](SortOptionType NameItem)
					{
						return SNew(STextBlock)
							.Text(NameItem->GetFText());
					})
					.OnSelectionChanged(this, &Self::HandleSortSelectionChanged)
					.ToolTipText(LOCTEXT("AddFilterToolTip", "Sort by a specific criteria."))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Sort", "Sort By"))
					]
				]

				// Refresh dialogues
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ToolTipText(LOCTEXT("RefreshToolTip", "Refreshes/Reloads the Dialogue Browser view."))
					.OnClicked(this, &Self::HandleOnRefresh)
					[
						MakeIconAndTextWidget(LOCTEXT("RefreshDialogues", "Refresh"),
							FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_ReloadAssetIcon))
					]
				]
			]

			// The Tree view
			+SVerticalBox::Slot()
			.AutoHeight()
			.FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(0.0f, 4.0f))
				[
					ParticipantsTreeView.ToSharedRef()
				]
			]
		]
	];

	RefreshTree(false);
}

void SDialogueBrowser::RefreshTree(bool bPreserveExpansion)
{
	// First, save off current expansion state
	TSet<TSharedPtr<FDialogueBrowserTreeNode>> OldExpansionState;
	if (bPreserveExpansion)
	{
		ParticipantsTreeView->GetExpandedItems(OldExpansionState);
	}

	RootTreeItem->ClearChildren();
	RootChildren.Empty();
	ParticipantsProperties.Empty();

	auto PopulateVariablePropertiesFromSearchResult = [](
		const TSharedPtr<FDialogueBrowserTreeVariableProperties> VariableProperties,
		const TSharedPtr<FDialogueSearchFoundResult> SearchResult, const FGuid& DialogueGuid)
	{
		if (VariableProperties->HasGraphNodeSet(DialogueGuid))
		{
			VariableProperties->GetMutableGraphNodeSet(DialogueGuid)
							  ->Append(SearchResult->GraphNodes);
		}
		if (VariableProperties->HasEdgeNodeSet(DialogueGuid))
		{
			VariableProperties->GetMutableEdgeNodeSet(DialogueGuid)
							  ->Append(SearchResult->EdgeNodes);
		}
	};

	// Build fast lookup structure for participants (the ParticipantsProperties)
	TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	for (const UDlgDialogue* Dialogue : Dialogues)
	{
		const FGuid DialogueGuid = Dialogue->GetDlgGuid();

		// Populate Participants
		TSet<FName> ParticipantsNames;
		Dialogue->GetAllParticipantNames(ParticipantsNames);
		for (const FName& ParticipantName : ParticipantsNames)
		{
			TSharedPtr<FDialogueBrowserTreeParticipantProperties>* ParticipantPropsPtr = ParticipantsProperties.Find(ParticipantName);
			TSharedPtr<FDialogueBrowserTreeParticipantProperties> ParticipantProps;
			if (ParticipantPropsPtr == nullptr)
			{
				// participant does not exist, create it
				const TSet<TWeakObjectPtr<const UDlgDialogue>> SetArgument{Dialogue};
				ParticipantProps = MakeShared<FDialogueBrowserTreeParticipantProperties>(SetArgument);
				ParticipantsProperties.Add(ParticipantName, ParticipantProps);
			}
			else
			{
				// exists
				ParticipantProps = *ParticipantPropsPtr;
				ParticipantProps->AddDialogue(Dialogue);
			}

			// Populate events
			TSet<FName> EventsNames;
			Dialogue->GetEvents(ParticipantName, EventsNames);
			for (const FName& EventName : EventsNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToEvent(EventName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForEventEventName(EventName, Dialogue),
					DialogueGuid);
			}

			// Populate conditions
			TSet<FName> ConditionNames;
			Dialogue->GetConditions(ParticipantName, ConditionNames);
			for (const FName& ConditionName : ConditionNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToCondition(ConditionName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForConditionEventCallName(ConditionName, Dialogue),
					DialogueGuid);
			}

			// Populate int variable names
			TSet<FName> IntVariableNames;
			Dialogue->GetIntNames(ParticipantName, IntVariableNames);
			for (const FName& IntVariableName : IntVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToIntVariable(IntVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForIntVariableName(IntVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate float variable names
			TSet<FName> FloatVariableNames;
			Dialogue->GetFloatNames(ParticipantName, FloatVariableNames);
			for (const FName& FloatVariableName : FloatVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToFloatVariable(FloatVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForFloatVariableName(FloatVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate bool variable names
			TSet<FName> BoolVariableNames;
			Dialogue->GetBoolNames(ParticipantName, BoolVariableNames);
			for (const FName& BoolVariableName : BoolVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToBoolVariable(BoolVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForBoolVariableName(BoolVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate FName variable names
			TSet<FName> FNameVariableNames;
			Dialogue->GetNameNames(ParticipantName, FNameVariableNames);
			for (const FName& NameVariableName : FNameVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToFNameVariable(NameVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForFNameVariableName(NameVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate UClass int variable names
			TSet<FName> ClassIntVariableNames;
			Dialogue->GetClassIntNames(ParticipantName, ClassIntVariableNames);
			for (const FName& IntVariableName : ClassIntVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassIntVariable(IntVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForClassIntVariableName(IntVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate UClass float variable names
			TSet<FName> ClassFloatVariableNames;
			Dialogue->GetClassFloatNames(ParticipantName, ClassFloatVariableNames);
			for (const FName& FloatVariableName : ClassFloatVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassFloatVariable(FloatVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForClassFloatVariableName(FloatVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate UClass bool variable names
			TSet<FName> ClassBoolVariableNames;
			Dialogue->GetClassBoolNames(ParticipantName, ClassBoolVariableNames);
			for (const FName& BoolVariableName : ClassBoolVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassBoolVariable(BoolVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForClassBoolVariableName(BoolVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate UClass FName variable names
			TSet<FName> ClassFNameVariableNames;
			Dialogue->GetClassNameNames(ParticipantName, ClassFNameVariableNames);
			for (const FName& NameVariableName : ClassFNameVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassFNameVariable(NameVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForClassFNameVariableName(NameVariableName, Dialogue),
					DialogueGuid);
			}

			// Populate UClass FText variable names
			TSet<FName> ClassFTextVariableNames;
			Dialogue->GetClassTextNames(ParticipantName, ClassFTextVariableNames);
			for (const FName& TextVariableName : ClassFTextVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassFTextVariable(TextVariableName, Dialogue),
					FDialogueSearchUtilities::GetGraphNodesForClassFTextVariableName(TextVariableName, Dialogue),
					DialogueGuid);
			}
		}
	}

	// Sort the properties
	TArray<FName> AllParticipants;
	for (const auto Elem : ParticipantsProperties)
	{
		AllParticipants.Add(Elem.Key);
		TSharedPtr<FDialogueBrowserTreeParticipantProperties> Property = Elem.Value;

		// sort
		Property->Sort();
	}

	// Sort the participant names
	if (SelectedSortOption->IsByName())
	{
		// Sort by name
		FDlgHelper::SortDefault(AllParticipants);
	}
	else
	{
		// Sort by dialogue references
		AllParticipants.Sort([this](const FName& A, const FName& B)
		{
			return FDialogueBrowserUtilities::PredicateSortByDialoguesNumDescending(A, B, ParticipantsProperties);
		});
	}

	// Build the tree
	for (const FName& Name : AllParticipants)
	{
		const TSharedPtr<FDialogueBrowserTreeNode> Participant =
			MakeShared<FDialogueBrowserTreeCategoryParticipantNode>(FText::FromName(Name), RootTreeItem, Name);

		BuildTreeViewItem(Participant);
		RootTreeItem->AddChild(Participant);
		RootTreeItem->AddChild(MakeShared<FDialogueBrowserTreeSeparatorNode>(RootTreeItem));
	}
	RootTreeItem->GetVisibleChildren(RootChildren);

	// Clear Previous states
	ParticipantsTreeView->ClearSelection();
	// Triggers RequestTreeRefresh
	ParticipantsTreeView->ClearExpandedItems();

	// Restore old expansion
	if (bPreserveExpansion && OldExpansionState.Num() > 0)
	{
		// Flattened tree
		TArray<TSharedPtr<FDialogueBrowserTreeNode>> AllNodes;
		RootTreeItem->GetAllNodes(AllNodes);

		// Expand to match the old state
		FDlgTreeViewHelper::RestoreTreeExpansionState<TSharedPtr<FDialogueBrowserTreeNode>>(ParticipantsTreeView,
			AllNodes, OldExpansionState, FDialogueBrowserUtilities::PredicateCompareDialogueTreeNode);
	}
}

void SDialogueBrowser::GenerateFilteredItems()
{
	if (FilterString.IsEmpty())
	{
		// No filtering, empty filter, restore original
		RefreshTree(false);
		return;
	}

	// Get all valid paths
	TArray<TArray<TSharedPtr<FDialogueBrowserTreeNode>>> OutPaths;
	RootTreeItem->FilterPathsToNodesThatContainText(FilterString, OutPaths);
	RootChildren.Empty();
	RootTreeItem->GetVisibleChildren(RootChildren);

	// Refresh, clear expansion
	ParticipantsTreeView->ClearExpandedItems(); // Triggers RequestTreeRefresh

	// Mark paths as expanded
	for (const TArray<TSharedPtr<FDialogueBrowserTreeNode>>& Path : OutPaths)
	{
		const int32 PathNum = Path.Num();
		for (int32 PathIndex = 0; PathIndex < PathNum; PathIndex++)
		{
			Path[PathIndex]->SetIsVisible(true);
			ParticipantsTreeView->SetItemExpansion(Path[PathIndex], true);
		}
	}

	// TODO tokens
}

TSharedRef<SWidget> SDialogueBrowser::GetFilterTextBoxWidget()
{
	// Is it cached?
	if (FilterTextBoxWidget.IsValid())
	{
		return FilterTextBoxWidget.ToSharedRef();
	}

	// Cache it
	FilterTextBoxWidget = SNew(SSearchBox)
		.HintText(LOCTEXT("SearchBoxHintText", "Search by Name"))
		.OnTextChanged(this, &Self::HandleSearchTextCommitted, ETextCommit::Default)
		.OnTextCommitted(this, &Self::HandleSearchTextCommitted)
		.SelectAllTextWhenFocused(false)
		.DelayChangeNotificationsWhileTyping(false);

	// Should return a valid widget
	return GetFilterTextBoxWidget();
}

void SDialogueBrowser::AddDialogueChildrenToItemFromProperty(const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
	 const TSharedPtr<FDialogueBrowserTreeVariableProperties>* PropertyPtr, const EDialogueTreeNodeTextType TextType)
{
	// List the dialogues that contain this event for this property
	TSet<TWeakObjectPtr<const UDlgDialogue>> Dialogues;
	if (PropertyPtr != nullptr)
	{
		Dialogues = (*PropertyPtr)->GetDialogues();
	}

	for (TWeakObjectPtr<const UDlgDialogue> Dialogue : Dialogues)
	{
		if (!Dialogue.IsValid())
		{
			continue;
		}

		const TSharedPtr<FDialogueBrowserTreeNode> DialogueItem =
			MakeShared<FDialogueBrowserTreeDialogueNode>(FText::FromName(Dialogue->GetDlgFName()), InItem, Dialogue);
		DialogueItem->SetTextType(TextType);
		InItem->AddChild(DialogueItem);
	}
}

void SDialogueBrowser::AddGraphNodeChildrenToItem(const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
								const TSet<TWeakObjectPtr<const UDialogueGraphNode>>& GraphNodes,
								const EDialogueTreeNodeTextType TextType)
{
	for (TWeakObjectPtr<const UDialogueGraphNode> GraphNode : GraphNodes)
	{
		if (!GraphNode.IsValid())
		{
			continue;
		}

		const FName Text = *FString::Printf(TEXT("%d"), GraphNode->GetDialogueNodeIndex());
		const TSharedPtr<FDialogueBrowserTreeNode> NodeItem =
			MakeShared<FDialogueBrowserTreeGraphNode>(FText::FromName(Text), InItem, GraphNode);
		NodeItem->SetTextType(TextType);
		InItem->AddInlineChild(NodeItem);
	}
}

void SDialogueBrowser::AddEdgeNodeChildrenToItem(const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
								const TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>& EdgeNodes,
								const EDialogueTreeNodeTextType TextType)
{
	for (TWeakObjectPtr<const UDialogueGraphNode_Edge> EdgeNode : EdgeNodes)
	{
		if (!EdgeNode.IsValid())
		{
			continue;
		}

		int32 FromParent = -1;
		int32 ToChild = -1;
		if (EdgeNode->HasParentNode())
		{
			FromParent = EdgeNode->GetParentNode()->GetDialogueNodeIndex();
		}
		if (EdgeNode->HasChildNode())
		{
			ToChild = EdgeNode->GetChildNode()->GetDialogueNodeIndex();
		}
		const FName Text = *FString::Printf(TEXT("%d -> %d"), FromParent, ToChild);
		const TSharedPtr<FDialogueBrowserTreeNode> NodeItem =
			MakeShared<FDialogueBrowserTreeEdgeNode>(FText::FromName(Text), InItem, EdgeNode);
		NodeItem->SetTextType(TextType);
		InItem->AddInlineChild(NodeItem);
	}
}

void SDialogueBrowser::AddGraphNodeBaseChildrenToItemFromProperty(const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
	const TSharedPtr<FDialogueBrowserTreeVariableProperties>* PropertyPtr,
	const EDialogueTreeNodeTextType GraphNodeTextType, const EDialogueTreeNodeTextType EdgeNodeTextType)
{
	TSharedPtr<FDialogueBrowserTreeDialogueNode> DialogueItem =
		StaticCastSharedPtr<FDialogueBrowserTreeDialogueNode>(InItem);
	if (!DialogueItem.IsValid())
	{
		return;
	}
	if (PropertyPtr == nullptr || !DialogueItem->GetDialogue().IsValid())
	{
		return;
	}

	const TSharedPtr<FDialogueBrowserTreeVariableProperties> Property = *PropertyPtr;
	const UDlgDialogue* Dialogue = DialogueItem->GetDialogue().Get();
	const FGuid DialogueGuid = Dialogue->GetDlgGuid();

	// Display the GraphNode
	if (Property->HasGraphNodeSet(DialogueGuid))
	{
		AddGraphNodeChildrenToItem(DialogueItem, Property->GetGraphNodeSet(DialogueGuid), GraphNodeTextType);
	}
	if (Property->HasEdgeNodeSet(DialogueGuid))
	{
		AddEdgeNodeChildrenToItem(DialogueItem, Property->GetEdgeNodeSet(DialogueGuid), EdgeNodeTextType);
	}
}

void SDialogueBrowser::AddVariableChildrenToItem(const TSharedPtr<FDialogueBrowserTreeNode>& Item,
	const TMap<FName, TSharedPtr<FDialogueBrowserTreeVariableProperties>>& Variables,
	const EDialogueTreeNodeTextType VariableType)
{
	for (const auto& Pair : Variables)
	{
		const FName VariableName = Pair.Key;
		const TSharedPtr<FDialogueBrowserTreeNode> ChildItem =
			MakeShared<FDialogueBrowserTreeVariableNode>(FText::FromName(VariableName), Item, VariableName);
		ChildItem->SetTextType(VariableType);
		Item->AddChild(ChildItem);
	}
}

void SDialogueBrowser::BuildTreeViewItem(const TSharedPtr<FDialogueBrowserTreeNode>& Item)
{
	const FName ParticipantName = Item->GetParentParticipantName();
	if (!ParticipantName.IsValid() || ParticipantName.IsNone())
	{
		return;
	}

	// Do we have the Participant cached?
	TSharedPtr<FDialogueBrowserTreeParticipantProperties>* ParticipantPropertiesPtr = ParticipantsProperties.Find(ParticipantName);
	if (ParticipantPropertiesPtr == nullptr)
	{
		return;
	}

	TSharedPtr<FDialogueBrowserTreeParticipantProperties> ParticipantProperties = *ParticipantPropertiesPtr;
	if (Item->IsCategory())
	{
		switch (Item->GetCategoryType())
		{
		case EDialogueTreeNodeCategoryType::Participant:
		{
			// Display the categories for the participant
			const bool bHideEmptyCategories = GetDefault<UDlgSystemSettings>()->bHideEmptyDialogueBrowserCategories;
			Item->SetChildren(MakeParticipantCategoriesChildren(Item, ParticipantProperties, bHideEmptyCategories));
			break;
		}
		case EDialogueTreeNodeCategoryType::Dialogue:
		{
			// Add the dialogues
			for (TWeakObjectPtr<const UDlgDialogue> Dialogue : ParticipantProperties->GetDialogues())
			{
				if (!Dialogue.IsValid())
				{
					continue;
				}

				const TSharedPtr<FDialogueBrowserTreeNode> DialogueItem =
					MakeShared<FDialogueBrowserTreeDialogueNode>(FText::FromName(Dialogue->GetDlgFName()), Item, Dialogue);
				DialogueItem->SetTextType(EDialogueTreeNodeTextType::ParticipantDialogue);
				Item->AddChild(DialogueItem);

			}
			break;
		}
		case EDialogueTreeNodeCategoryType::Event:
		{
			// Display the events for this category
			for (const auto& Pair : ParticipantProperties->GetEvents())
			{
				const TSharedPtr<FDialogueBrowserTreeNode> EventItem =
					MakeShared<FDialogueBrowserTreeVariableNode>(FText::FromName(Pair.Key), Item, Pair.Key);
				EventItem->SetTextType(EDialogueTreeNodeTextType::ParticipantEvent);
				Item->AddChild(EventItem);
			}
			break;
		}
		case EDialogueTreeNodeCategoryType::Condition:
		{
			for (const auto& Pair : ParticipantProperties->GetConditions())
			{
				const TSharedPtr<FDialogueBrowserTreeNode> ConditionItem =
					MakeShared<FDialogueBrowserTreeVariableNode>(FText::FromName(Pair.Key), Item, Pair.Key);
				ConditionItem->SetTextType(EDialogueTreeNodeTextType::ParticipantCondition);
				Item->AddChild(ConditionItem);
			}
			break;
		}

		case EDialogueTreeNodeCategoryType::Variable:
		{
			// Only display the categories if the Participant has at least one variable.
			if (ParticipantProperties->HasVariables())
			{
				const bool bHideEmptyCategories = GetDefault<UDlgSystemSettings>()->bHideEmptyDialogueBrowserCategories;
				Item->SetChildren(MakeVariableCategoriesChildren(Item, ParticipantProperties, bHideEmptyCategories));
			}
			break;
		}
		case EDialogueTreeNodeCategoryType::VariableInt:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetIntegers(), EDialogueTreeNodeTextType::ParticipantVariableInt);
			break;
		case EDialogueTreeNodeCategoryType::VariableFloat:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetFloats(), EDialogueTreeNodeTextType::ParticipantVariableFloat);
			break;
		case EDialogueTreeNodeCategoryType::VariableBool:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetBools(), EDialogueTreeNodeTextType::ParticipantVariableBool);
			break;
		case EDialogueTreeNodeCategoryType::VariableFName:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetFNames(), EDialogueTreeNodeTextType::ParticipantVariableFName);
			break;

		case EDialogueTreeNodeCategoryType::ClassVariable:
		{
			// Only display the categories if the Participant has at least one class variable.
			if (ParticipantProperties->HasClassVariables())
			{
				const bool bHideEmptyCategories = GetDefault<UDlgSystemSettings>()->bHideEmptyDialogueBrowserCategories;
				Item->SetChildren(MakeClassVariableCategoriesChildren(Item, ParticipantProperties, bHideEmptyCategories));
			}
			break;
		}
		case EDialogueTreeNodeCategoryType::ClassVariableInt:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetClassIntegers(), EDialogueTreeNodeTextType::ParticipantClassVariableInt);
			break;
		case EDialogueTreeNodeCategoryType::ClassVariableFloat:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetClassFloats(), EDialogueTreeNodeTextType::ParticipantClassVariableFloat);
			break;
		case EDialogueTreeNodeCategoryType::ClassVariableBool:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetClassBools(), EDialogueTreeNodeTextType::ParticipantClassVariableBool);
			break;
		case EDialogueTreeNodeCategoryType::ClassVariableFName:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetClassFNames(), EDialogueTreeNodeTextType::ParticipantClassVariableFName);
			break;
		case EDialogueTreeNodeCategoryType::ClassVariableFText:
			AddVariableChildrenToItem(Item, ParticipantProperties->GetClassFTexts(), EDialogueTreeNodeTextType::ParticipantClassVariableFText);
			break;

		default:
			break;
		}
	}
	else if (Item->IsText())
	{
		switch (Item->GetTextType())
		{
		case EDialogueTreeNodeTextType::ParticipantEvent:
			// List the dialogues that contain this event for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetEvents().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::EventDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantCondition:
			// List the dialogues that contain this condition for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetConditions().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::ConditionDialogue);
			break;

		case EDialogueTreeNodeTextType::ParticipantVariableInt:
			// List the dialogues that contain this int variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetIntegers().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::IntVariableDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantVariableFloat:
			// List the dialogues that contain this float variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetFloats().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FloatVariableDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantVariableBool:
			// List the dialogues that contain this bool variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetBools().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::BoolVariableDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantVariableFName:
			// List the dialogues that contain this Fname variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetFNames().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FNameVariableDialogue);
			break;

		case EDialogueTreeNodeTextType::ParticipantClassVariableInt:
			// List the dialogues that contain this UClass int variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassIntegers().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::IntClassVariableDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantClassVariableFloat:
			// List the dialogues that contain this UClass float variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassFloats().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FloatClassVariableDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantClassVariableBool:
			// List the dialogues that contain this UClass bool variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassBools().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::BoolClassVariableDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantClassVariableFName:
			// List the dialogues that contain this UClass Fname variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassFNames().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FNameClassVariableDialogue);
			break;
		case EDialogueTreeNodeTextType::ParticipantClassVariableFText:
			// List the dialogues that contain this UClass FText variable for this participant
			AddDialogueChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassFTexts().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FTextClassVariableDialogue);
			break;

		case EDialogueTreeNodeTextType::EventDialogue:
			// List the graph nodes for the dialogue that contains this event
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetEvents().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::EventGraphNode, EDialogueTreeNodeTextType::EventGraphNode);
			break;
		case EDialogueTreeNodeTextType::ConditionDialogue:
			// List the graph nodes for the dialogue that contains this condition
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetConditions().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::ConditionGraphNode, EDialogueTreeNodeTextType::ConditionEdgeNode);
			break;

		case EDialogueTreeNodeTextType::IntVariableDialogue:
			// List the graph nodes for the dialogue that contains this int variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetIntegers().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::IntVariableGraphNode, EDialogueTreeNodeTextType::IntVariableEdgeNode);
			break;
		case EDialogueTreeNodeTextType::FloatVariableDialogue:
			// List the graph nodes for the dialogue that contains this float variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetFloats().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FloatVariableGraphNode, EDialogueTreeNodeTextType::FloatVariableEdgeNode);
			break;
		case EDialogueTreeNodeTextType::BoolVariableDialogue:
			// List the graph nodes for the dialogue that contains this bool variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetBools().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::BoolVariableGraphNode, EDialogueTreeNodeTextType::BoolVariableEdgeNode);
			break;
		case EDialogueTreeNodeTextType::FNameVariableDialogue:
			// List the graph nodes for the dialogue that contains this FName variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetFNames().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FNameVariableGraphNode, EDialogueTreeNodeTextType::FNameVariableEdgeNode);
			break;

		case EDialogueTreeNodeTextType::IntClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass int variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassIntegers().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::IntVariableGraphNode, EDialogueTreeNodeTextType::IntVariableEdgeNode);
			break;
		case EDialogueTreeNodeTextType::FloatClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass float variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassFloats().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FloatVariableGraphNode, EDialogueTreeNodeTextType::FloatVariableEdgeNode);
			break;
		case EDialogueTreeNodeTextType::BoolClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass bool variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassBools().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::BoolVariableGraphNode, EDialogueTreeNodeTextType::BoolVariableEdgeNode);
			break;
		case EDialogueTreeNodeTextType::FNameClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass FName variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassFNames().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FNameVariableGraphNode, EDialogueTreeNodeTextType::FNameVariableEdgeNode);
			break;

		case EDialogueTreeNodeTextType::FTextClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass FText variable
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetClassFTexts().Find(Item->GetParentVariableName()),
				EDialogueTreeNodeTextType::FTextVariableGraphNode, EDialogueTreeNodeTextType::FTextVariableEdgeNode);
			break;

		default:
			break;
		}
	}

	// Recursively call on children
	for (const TSharedPtr<FDialogueBrowserTreeNode>& ChildItem : Item->GetChildren())
	{
		BuildTreeViewItem(ChildItem);
	}

	// The same for the inline children, handled separately.
	for (const TSharedPtr<FDialogueBrowserTreeNode>& ChildItem : Item->GetInlineChildren())
	{
		BuildTreeViewItem(ChildItem);
	}
}

TSharedRef<SWidget> SDialogueBrowser::MakeButtonWidgetForGraphNodes(const TArray<TSharedPtr<FDialogueBrowserTreeNode>>& InChildren)
{
	TSharedPtr<SWrapBox> Buttons = SNew(SWrapBox)
			.PreferredWidth(600.f);

	// Constructs [Node 1] [Node 2]
	const FText GraphNodeTooltip = LOCTEXT("JumpToNodeTipGraphNode", "Opens the Dialogue Editor and jumps to the Node");
	const FText EdgeNodeTooltip = LOCTEXT("JumpToNodeTipEdgeNode", "Opens the Dialogue Editor and jumps to the Edge");
	for (const TSharedPtr<FDialogueBrowserTreeNode>& ChildItem : InChildren)
	{
		const bool bIsEdgeNodeText =  ChildItem->IsEdgeNodeText();
		if (ChildItem->IsGraphNodeText() || bIsEdgeNodeText)
		{
			Buttons->AddSlot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(2.f, 0.f, 0.f, 0.f)
				[
					// Jump To node
					SNew(SButton)
					.ToolTipText(bIsEdgeNodeText ? EdgeNodeTooltip : GraphNodeTooltip)
					.OnClicked(ChildItem.Get(), &FDialogueBrowserTreeNode::OnClick)
					[
						SNew(STextBlock)
						.Text(ChildItem->GetDisplayText())
						.Font(FEditorStyle::GetFontStyle(TEXT("FontAwesome.10")))
					]
				];
		}
	}

	return Buttons.ToSharedRef();
}

TSharedRef<SWidget> SDialogueBrowser::MakeInlineWidget(const TSharedPtr<FDialogueBrowserTreeNode>& InItem)
{
	if (!InItem.IsValid())
	{
		return SMissingWidget::MakeMissingWidget();
	}
	if (!InItem->HasInlineChildren() || !InItem->IsText())
	{
		return SMissingWidget::MakeMissingWidget();
	}

	if (InItem->IsDialogueText())
	{
		// Display the [Dialogue] [Node1] [Node2] [Node3] [Jump to Content Browser] [Open Dialogue]
		return SNew(SHorizontalBox)
			// Icon and Dialogue Name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				MakeIconAndTextWidget(InItem->GetDisplayText(), FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_DialogueClassThumbnail))
			]

			// Graph Nodes
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				MakeButtonWidgetForGraphNodes(InItem->GetInlineChildren())
			]

			// Buttons on the right side
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				MakeButtonsWidgetForDialogue(InItem)
			];
	}

	return SMissingWidget::MakeMissingWidget();
}

TSharedRef<SWidget> SDialogueBrowser::MakeButtonsWidgetForDialogue(const TSharedPtr<FDialogueBrowserTreeNode>& InItem)
{
	return SNew(SHorizontalBox)
		// Find in Content Browser
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			.ToolTipText(LOCTEXT("FindInContentBrowserToolTip", "Find the Dialogue in the Context Browser"))
			.OnClicked(this, &Self::FindInContentBrowserForItem, InItem)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.WidthOverride(16)
				.HeightOverride(16)
				[
					SNew(SImage)
					.Image(FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_FindAssetIcon))
				]
			]
		]

		// Open Editor
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			.ToolTipText(LOCTEXT("OpenDialogueToolTip", "Opens the dialogue editor."))
			.OnClicked(InItem.Get(), &FDialogueBrowserTreeNode::OnClick)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.WidthOverride(16)
				.HeightOverride(16)
				[
					SNew(SImage)
					.Image(FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_OpenAssetIcon))
				]
			]
		];
}

void SDialogueBrowser::HandleSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	// Trim and sanitized the filter text (so that it more likely matches)
	FilterString = FText::TrimPrecedingAndTrailing(InText).ToString();
	GenerateFilteredItems();
//	RefreshTree(false);
}

TSharedRef<ITableRow> SDialogueBrowser::HandleGenerateRow(TSharedPtr<FDialogueBrowserTreeNode> InItem,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// Build row
	TSharedPtr<STableRow<TSharedPtr<FDialogueBrowserTreeNode>>> TableRow;
	FMargin RowPadding = FMargin(2.f, 2.f);
	const bool bIsCategory = InItem->IsCategory();
	const bool bIsSeparator = InItem->IsSeparator();

	if (bIsCategory)
	{
		TableRow = SNew(SCategoryHeaderTableRow<TSharedPtr<FDialogueBrowserTreeNode>>, OwnerTable)
				.Visibility(InItem->IsVisible() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	else
	{
		TableRow = SNew(STableRow<TSharedPtr<FDialogueBrowserTreeNode>>, OwnerTable)
			.Padding(1.0f)
			.Visibility(InItem->IsVisible() ? EVisibility::Visible : EVisibility::Collapsed)
			.ShowSelection(!bIsSeparator);
	}

	// Not visible.
	// NOTE should not be used normally, but only after the tree view is dirty.
	if (!InItem->IsVisible())
	{
		checkNoEntry();
		return TableRow.ToSharedRef();
	}

	// Default row content
	TSharedPtr<STextBlock> DefaultTextBlock = SNew(STextBlock)
			.Text(InItem->GetDisplayText())
			.HighlightText(this, &Self::GetFilterText)
			.Font(FEditorStyle::GetFontStyle(TEXT("FontAwesome.10")));

	TSharedPtr<SWidget> RowContent = DefaultTextBlock;
	TSharedPtr<SHorizontalBox> RowContainer;
	TableRow->SetRowContent(SAssignNew(RowContainer, SHorizontalBox));

	// Build content
	if (bIsSeparator)
	{
		RowPadding = FMargin(0);
		RowContent = SNew(SVerticalBox)
			.Visibility(EVisibility::HitTestInvisible)

			+SVerticalBox::Slot()
			.AutoHeight()
			// Add some empty space before the line, and a tiny bit after it
			.Padding(0.0f, 5.f, 0.0f, 5.f)
			[
				SNew(SBorder)

				// We'll use the border's padding to actually create the horizontal line
				.Padding(FEditorStyle::GetMargin(TEXT("Menu.Separator.Padding")))

				// Separator graphic
				.BorderImage(FEditorStyle::GetBrush(TEXT("Menu.Separator")))
			];
	}
	else if (bIsCategory)
	{
		if (InItem->GetCategoryType() == EDialogueTreeNodeCategoryType::Participant)
		{
			int32 DialogueReferences = 0;
			TSharedPtr<FDialogueBrowserTreeParticipantProperties>* ParticipantPropertiesPtr =
				ParticipantsProperties.Find(InItem->GetParentParticipantName());
			if (ParticipantPropertiesPtr)
			{
				DialogueReferences = (*ParticipantPropertiesPtr)->GetDialogues().Num();
			}

			RowContent = SNew(SHorizontalBox)

				// Name of participant
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(FEditorStyle::GetFontStyle(TEXT("FontAwesome.16")))
					.Text(InItem->GetDisplayText())
					.HighlightText(this, &Self::GetFilterText)
				]

				// Number of dialogue references
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(FEditorStyle::GetFontStyle(TEXT("FontAwesome.9")))
					.Text(FText::FromString(FString::Printf(TEXT("Dialogue references %d"), DialogueReferences)))
				];
		}
		else
		{
			RowContent = SNew(STextBlock)
				.Font(FEditorStyle::GetFontStyle(TEXT("FontAwesome.12")))
				.Text(InItem->GetDisplayText())
				.HighlightText(this, &Self::GetFilterText);
		}
	}
	else if (InItem->IsText())
	{
		if (InItem->HasInlineChildren())
		{
			RowContent = MakeInlineWidget(InItem);
		}
		else if (InItem->IsDialogueText())
		{
			RowContent = SNew(SHorizontalBox)

				// Icon and Dialogue Name
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					MakeIconAndTextWidget(InItem->GetDisplayText(),
						FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_DialogueClassThumbnail))
				]

				// Buttons on the right side
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					MakeButtonsWidgetForDialogue(InItem)
				];
		}
		else if (InItem->IsEventText())
		{
			RowContent = MakeIconAndTextWidget(InItem->GetDisplayText(),
				FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_EventIcon));
		}
		else if (InItem->IsConditionText())
		{
			RowContent = MakeIconAndTextWidget(InItem->GetDisplayText(),
				FDialogueStyle::Get()->GetBrush(FDialogueStyle::PROPERTY_ConditionIcon));
		}
		else if (InItem->IsGraphNodeText() || InItem->IsEdgeNodeText())
		{
			RowContent = SNew(SHorizontalBox)

				// Text
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					DefaultTextBlock.ToSharedRef()
				]

				// Jump To node
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ToolTipText(LOCTEXT("JumpToNodeTip", "Opens the Editor for the Dialogue and jumps to the node"))
					.OnClicked(InItem.Get(), &FDialogueBrowserTreeNode::OnClick)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("JumpToNode", "Jump"))
					]
				];
		}
	}
	else
	{
		// did we miss something?
	}

	RowContainer->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Right)
		[
			SNew(SExpanderArrow, TableRow)
		];

	RowContainer->AddSlot()
		.FillWidth(1.0)
		.Padding(RowPadding)
		[
			RowContent.ToSharedRef()
		];

	return TableRow.ToSharedRef();
}

void SDialogueBrowser::HandleGetChildren(TSharedPtr<FDialogueBrowserTreeNode> InItem, TArray<TSharedPtr<FDialogueBrowserTreeNode>>& OutChildren)
{
	if (!InItem.IsValid() || InItem->IsSeparator())
	{
		return;
	}
	if (InItem->HasChildren())
	{
		InItem->GetVisibleChildren(OutChildren);
	}
}

void SDialogueBrowser::HandleTreeSelectionChanged(TSharedPtr<FDialogueBrowserTreeNode> NewValue, ESelectInfo::Type SelectInfo)
{
	// Ignore
}

void SDialogueBrowser::HandleDoubleClick(TSharedPtr<FDialogueBrowserTreeNode> InItem)
{
	if (!InItem.IsValid())
	{
		return;
	}

	if (InItem->IsText())
	{
		InItem->OnClick();
	}

	// Expand on double click
	if (InItem->HasChildren())
	{
		ParticipantsTreeView->SetItemExpansion(InItem, !ParticipantsTreeView->IsItemExpanded(InItem));
	}
}

void SDialogueBrowser::HandleSetExpansionRecursive(TSharedPtr<FDialogueBrowserTreeNode> InItem, bool bInIsItemExpanded)
{
	if (InItem.IsValid() && InItem->HasChildren())
	{
		ParticipantsTreeView->SetItemExpansion(InItem, bInIsItemExpanded);
		for (const TSharedPtr<FDialogueBrowserTreeNode> Child : InItem->GetChildren())
		{
			HandleSetExpansionRecursive(Child, bInIsItemExpanded);
		}
	}
}

void SDialogueBrowser::HandleSortSelectionChanged(SortOptionType Selection, ESelectInfo::Type SelectInfo)
{
	if (Selection.IsValid())
	{
		SelectedSortOption = Selection;
		RefreshTree(true);
	}
}

FReply SDialogueBrowser::FindInContentBrowserForItem(TSharedPtr<FDialogueBrowserTreeNode> InItem)
{
	TSharedPtr<FDialogueBrowserTreeDialogueNode> DialogueItem =
		StaticCastSharedPtr<FDialogueBrowserTreeDialogueNode>(InItem);
	if (!DialogueItem.IsValid())
	{
		return FReply::Unhandled();
	}

	static constexpr bool bFocusContentBrowser = true;
	if (GEditor && DialogueItem->GetDialogue().IsValid())
	{
		TArray<UObject*> ObjectsToSyncTo{const_cast<UDlgDialogue*>(DialogueItem->GetDialogue().Get())};
		GEditor->SyncBrowserToObjects(ObjectsToSyncTo, bFocusContentBrowser);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

TArray<TSharedPtr<FDialogueBrowserTreeNode>> SDialogueBrowser::MakeParticipantCategoriesChildren(
	const TSharedPtr<FDialogueBrowserTreeNode>& Parent,
	const TSharedPtr<FDialogueBrowserTreeParticipantProperties>& ParticipantProperties,
	bool bHideEmptyCategories) const
{
	TArray<TSharedPtr<FDialogueBrowserTreeNode>> Categories;
	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasDialogues()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Dialogues")), Parent, EDialogueTreeNodeCategoryType::Dialogue
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasEvents()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Events")), Parent, EDialogueTreeNodeCategoryType::Event
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasConditions()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Conditions")), Parent, EDialogueTreeNodeCategoryType::Condition
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasVariables()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Variables")), Parent, EDialogueTreeNodeCategoryType::Variable
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassVariables()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Class Variables")), Parent, EDialogueTreeNodeCategoryType::ClassVariable
		);
		Categories.Add(Category);
	}
	return Categories;
}

TArray<TSharedPtr<FDialogueBrowserTreeNode>> SDialogueBrowser::MakeVariableCategoriesChildren(
	const TSharedPtr<FDialogueBrowserTreeNode>& Parent,
	const TSharedPtr<FDialogueBrowserTreeParticipantProperties>& ParticipantProperties,
	bool bHideEmptyCategories) const
{
	TArray<TSharedPtr<FDialogueBrowserTreeNode>> Categories;
	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasIntegers()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Integers")), Parent, EDialogueTreeNodeCategoryType::VariableInt
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasFloats()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Floats")), Parent, EDialogueTreeNodeCategoryType::VariableFloat
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasBools()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Bools")), Parent, EDialogueTreeNodeCategoryType::VariableBool
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasFNames()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("FNames")), Parent, EDialogueTreeNodeCategoryType::VariableFName
		);
		Categories.Add(Category);
	}

	return Categories;
}

TArray<TSharedPtr<FDialogueBrowserTreeNode>> SDialogueBrowser::MakeClassVariableCategoriesChildren(
	const TSharedPtr<FDialogueBrowserTreeNode>& Parent,
	const TSharedPtr<FDialogueBrowserTreeParticipantProperties>& ParticipantProperties,
	bool bHideEmptyCategories) const
{
	TArray<TSharedPtr<FDialogueBrowserTreeNode>> Categories;

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassIntegers()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Class Integers")), Parent, EDialogueTreeNodeCategoryType::ClassVariableInt
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassFloats()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Class Floats")), Parent, EDialogueTreeNodeCategoryType::ClassVariableFloat
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassBools()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Class Bools")), Parent, EDialogueTreeNodeCategoryType::ClassVariableBool
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassFNames()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Class FNames")), Parent, EDialogueTreeNodeCategoryType::ClassVariableFName
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassFTexts()))
	{
		TSharedPtr<FDialogueBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Class FTexts")), Parent, EDialogueTreeNodeCategoryType::ClassVariableFText
		);
		Categories.Add(Category);
	}

	return Categories;
}

TSharedRef<SHorizontalBox> SDialogueBrowser::MakeIconAndTextWidget(const FText& InText,
	const FSlateBrush* IconBrush, int32 IconSize)
{
	return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.WidthOverride(IconSize)
			.HeightOverride(IconSize)
			[
				SNew(SImage)
				.Image(IconBrush)
			]
		]

		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(2.f, 0.f)
		[
			SNew(STextBlock)
			.Text(InText)
			.HighlightText(this, &Self::GetFilterText)
		];
}

TSharedRef<SWidget> SDialogueBrowser::FillViewOptionsEntries()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("HideEmptyCategories", "Hide empty categories"),
		LOCTEXT("HideEmptyCategories_ToolTip", "Hides categories that do not have any children"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				UDlgSystemSettings* Settings = GetMutableDefault<UDlgSystemSettings>();
				Settings->SetHideEmptyDialogueBrowserCategories(!Settings->bHideEmptyDialogueBrowserCategories);
				RefreshTree(true);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([]() -> bool
			{
				return GetDefault<UDlgSystemSettings>()->bHideEmptyDialogueBrowserCategories;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);

	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
