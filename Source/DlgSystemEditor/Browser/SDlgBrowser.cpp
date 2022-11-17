// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgBrowser.h"

#include "Editor.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SMissingWidget.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"

#include "DlgSystem/DlgManager.h"
#include "DlgSystem/DlgDialogue.h"
#include "DlgSystemEditor/DlgStyle.h"
#include "DlgSystemEditor/Search/DlgSearchUtilities.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgBrowserUtilities.h"
#include "SourceCodeNavigation.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "DlgSystem/TreeViewHelpers/DlgTreeViewHelper.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "SDlgBrowser"
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

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
			.Style(FNYAppStyle::Get(), "DetailsView.TreeView.TableRow")
			.ShowSelection(false),
			InOwnerTableView
		);
	}

	const FSlateBrush* GetBackgroundImage() const
	{
		if (STableRow<ItemType>::IsHovered())
		{
			return STableRow<ItemType>::IsItemExpanded() ? FNYAppStyle::GetBrush("DetailsView.CategoryTop_Hovered") : FNYAppStyle::GetBrush("DetailsView.CollapsedCategory_Hovered");
		}

		return STableRow<ItemType>::IsItemExpanded() ? FNYAppStyle::GetBrush("DetailsView.CategoryTop") : FNYAppStyle::GetBrush("DetailsView.CollapsedCategory");
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
// SDlgBrowser
void SDlgBrowser::Construct(const FArguments& InArgs)
{
	DefaultSortOption = MakeShared<FDlgBrowserSortOption>(EDlgBrowserSortOption::DialogueReferences, TEXT("Dialogue References"));
	SelectedSortOption = DefaultSortOption;
	SortOptions = {
		DefaultSortOption,
		MakeShared<FDlgBrowserSortOption>(EDlgBrowserSortOption::Name, TEXT("Name"))
	};

	RootTreeItem = MakeShared<FDialogueBrowserTreeRootNode>();
	ParticipantsTreeView = SNew(STreeView<TSharedPtr<FDlgBrowserTreeNode>>)
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
		.BorderImage(FNYAppStyle::GetBrush("ToolPanel.GroupBorder"))
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
					.ComboButtonStyle(FNYAppStyle::Get(), "GenericFilters.ComboButtonStyle")
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
							.TextStyle(FNYAppStyle::Get(), "GenericFilters.TextStyle")
							.Font(FNYAppStyle::Get().GetFontStyle("FontAwesome.9"))
							.Text(FText::FromString(FString(TEXT("\xf0b0"))) /*fa-filter*/)
						]
						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FNYAppStyle::Get(), "GenericFilters.TextStyle")
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
						MakeIconAndTextWidget(
							LOCTEXT("RefreshDialogues", "Refresh"),
							FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_ReloadAssetIcon)
						)
					]
				]
			]

			// The Tree view
			+SVerticalBox::Slot()
			.AutoHeight()
			.FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(FNYAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(0.0f, 4.0f))
				[
					ParticipantsTreeView.ToSharedRef()
				]
			]
		]
	];

	RefreshTree(false);
}

void SDlgBrowser::RefreshTree(bool bPreserveExpansion)
{
	// First, save off current expansion state
	TSet<TSharedPtr<FDlgBrowserTreeNode>> OldExpansionState;
	if (bPreserveExpansion)
	{
		ParticipantsTreeView->GetExpandedItems(OldExpansionState);
	}

	RootTreeItem->ClearChildren();
	RootChildren.Empty();
	ParticipantsProperties.Empty();

	auto PopulateVariablePropertiesFromSearchResult = [](
		const TSharedPtr<FDlgBrowserTreeVariableProperties> VariableProperties,
		const TSharedPtr<FDlgSearchFoundResult> SearchResult,
		const FGuid& DialogueGUID
	)
	{
		if (VariableProperties->HasGraphNodeSet(DialogueGUID))
		{
			VariableProperties
				->GetMutableGraphNodeSet(DialogueGUID)
				->Append(SearchResult->GraphNodes);
		}
		if (VariableProperties->HasEdgeNodeSet(DialogueGUID))
		{
			VariableProperties
				->GetMutableEdgeNodeSet(DialogueGUID)
				->Append(SearchResult->EdgeNodes);
		}
	};

	// Build fast lookup structure for participants (the ParticipantsProperties)
	TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	for (const UDlgDialogue* Dialogue : Dialogues)
	{
		const FGuid DialogueGUID = Dialogue->GetGUID();

		// Populate Participants
		TSet<FName> ParticipantsNames = Dialogue->GetParticipantNames();
		for (const FName& ParticipantName : ParticipantsNames)
		{
			TSharedPtr<FDlgBrowserTreeParticipantProperties>* ParticipantPropsPtr = ParticipantsProperties.Find(ParticipantName);
			TSharedPtr<FDlgBrowserTreeParticipantProperties> ParticipantProps;
			if (ParticipantPropsPtr == nullptr)
			{
				// participant does not exist, create it
				const TSet<TWeakObjectPtr<const UDlgDialogue>> SetArgument{Dialogue};
				ParticipantProps = MakeShared<FDlgBrowserTreeParticipantProperties>(SetArgument);
				ParticipantsProperties.Add(ParticipantName, ParticipantProps);
			}
			else
			{
				// exists
				ParticipantProps = *ParticipantPropsPtr;
				ParticipantProps->AddDialogue(Dialogue);
			}

			// Populate events
			const TSet<FName> EventsNames = Dialogue->GetParticipantEventNames(ParticipantName);
			for (const FName& EventName : EventsNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToEvent(EventName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForEventEventName(EventName, Dialogue),
					DialogueGUID
				);
			}

			// Populate Custom events
			const TSet<UClass*> CustomEventsClasses = Dialogue->GetParticipantCustomEvents(ParticipantName);
			for (UClass* EventClass : CustomEventsClasses)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToCustomEvent(EventClass, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForCustomEvent(EventClass, Dialogue),
					DialogueGUID
				);
			}

			// Populate conditions
			const TSet<FName> ConditionNames = Dialogue->GetParticipantConditionNames(ParticipantName);
			for (const FName& ConditionName : ConditionNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToCondition(ConditionName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForConditionEventCallName(ConditionName, Dialogue),
					DialogueGUID
				);
			}

			// Populate int variable names
			const TSet<FName> IntVariableNames = Dialogue->GetParticipantIntNames(ParticipantName);
			for (const FName& IntVariableName : IntVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToIntVariable(IntVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForIntVariableName(IntVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate float variable names
			const TSet<FName> FloatVariableNames = Dialogue->GetParticipantFloatNames(ParticipantName);
			for (const FName& FloatVariableName : FloatVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToFloatVariable(FloatVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForFloatVariableName(FloatVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate bool variable names
			const TSet<FName> BoolVariableNames = Dialogue->GetParticipantBoolNames(ParticipantName);
			for (const FName& BoolVariableName : BoolVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToBoolVariable(BoolVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForBoolVariableName(BoolVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate FName variable names
			const TSet<FName> FNameVariableNames = Dialogue->GetParticipantFNameNames(ParticipantName);
			for (const FName& NameVariableName : FNameVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToFNameVariable(NameVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForFNameVariableName(NameVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate UClass int variable names
			const TSet<FName> ClassIntVariableNames = Dialogue->GetParticipantClassIntNames(ParticipantName);
			for (const FName& IntVariableName : ClassIntVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassIntVariable(IntVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForClassIntVariableName(IntVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate UClass float variable names
			const TSet<FName> ClassFloatVariableNames = Dialogue->GetParticipantClassFloatNames(ParticipantName);
			for (const FName& FloatVariableName : ClassFloatVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassFloatVariable(FloatVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForClassFloatVariableName(FloatVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate UClass bool variable names
			const TSet<FName> ClassBoolVariableNames = Dialogue->GetParticipantClassBoolNames(ParticipantName);
			for (const FName& BoolVariableName : ClassBoolVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassBoolVariable(BoolVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForClassBoolVariableName(BoolVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate UClass FName variable names
			const TSet<FName> ClassFNameVariableNames = Dialogue->GetParticipantClassFNameNames(ParticipantName);
			for (const FName& NameVariableName : ClassFNameVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassFNameVariable(NameVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForClassFNameVariableName(NameVariableName, Dialogue),
					DialogueGUID
				);
			}

			// Populate UClass FText variable names
			const TSet<FName> ClassFTextVariableNames = Dialogue->GetParticipantClassFTextNames(ParticipantName);
			for (const FName& TextVariableName : ClassFTextVariableNames)
			{
				PopulateVariablePropertiesFromSearchResult(
					ParticipantProps->AddDialogueToClassFTextVariable(TextVariableName, Dialogue),
					FDlgSearchUtilities::GetGraphNodesForClassFTextVariableName(TextVariableName, Dialogue),
					DialogueGUID
				);
			}
		}
	}

	// Sort the properties
	TArray<FName> AllParticipants;
	for (const auto& Elem : ParticipantsProperties)
	{
		AllParticipants.Add(Elem.Key);
		TSharedPtr<FDlgBrowserTreeParticipantProperties> Property = Elem.Value;

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
			return FDlgBrowserUtilities::PredicateSortByDialoguesNumDescending(A, B, ParticipantsProperties);
		});
	}

	// Build the tree
	for (const FName& Name : AllParticipants)
	{
		const TSharedPtr<FDlgBrowserTreeNode> Participant =
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
		TArray<TSharedPtr<FDlgBrowserTreeNode>> AllNodes;
		RootTreeItem->GetAllNodes(AllNodes);

		// Expand to match the old state
		FDlgTreeViewHelper::RestoreTreeExpansionState<TSharedPtr<FDlgBrowserTreeNode>>(
			ParticipantsTreeView,
			AllNodes,
			OldExpansionState,
			FDlgBrowserUtilities::PredicateCompareDialogueTreeNode
		);
	}
}

void SDlgBrowser::GenerateFilteredItems()
{
	if (FilterString.IsEmpty())
	{
		// No filtering, empty filter, restore original
		RefreshTree(false);
		return;
	}

	// Get all valid paths
	TArray<TArray<TSharedPtr<FDlgBrowserTreeNode>>> OutPaths;
	RootTreeItem->FilterPathsToNodesThatContainText(FilterString, OutPaths);
	RootChildren.Empty();
	RootTreeItem->GetVisibleChildren(RootChildren);

	// Refresh, clear expansion
	ParticipantsTreeView->ClearExpandedItems(); // Triggers RequestTreeRefresh

	// Mark paths as expanded
	for (const TArray<TSharedPtr<FDlgBrowserTreeNode>>& Path : OutPaths)
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

TSharedRef<SWidget> SDlgBrowser::GetFilterTextBoxWidget()
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

void SDlgBrowser::AddDialogueChildrenToItemFromProperty(
	const TSharedPtr<FDlgBrowserTreeNode>& InItem,
	const TSharedPtr<FDlgBrowserTreeVariableProperties>* PropertyPtr,
	EDlgTreeNodeTextType TextType
)
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

		const TSharedPtr<FDlgBrowserTreeNode> DialogueItem = MakeShared<FDialogueBrowserTreeDialogueNode>(
			FText::FromName(Dialogue->GetDialogueFName()),
			InItem,
			Dialogue
		);
		DialogueItem->SetTextType(TextType);
		InItem->AddChild(DialogueItem);
	}
}

void SDlgBrowser::AddGraphNodeChildrenToItem(
	const TSharedPtr<FDlgBrowserTreeNode>& InItem,
	const TSet<TWeakObjectPtr<const UDialogueGraphNode>>& GraphNodes,
	EDlgTreeNodeTextType TextType
)
{
	for (TWeakObjectPtr<const UDialogueGraphNode> GraphNode : GraphNodes)
	{
		if (!GraphNode.IsValid())
		{
			continue;
		}

		const FName Text = *FString::Printf(TEXT("%d"), GraphNode->GetDialogueNodeIndex());
		const TSharedPtr<FDlgBrowserTreeNode> NodeItem =
			MakeShared<FDialogueBrowserTreeGraphNode>(FText::FromName(Text), InItem, GraphNode);
		NodeItem->SetTextType(TextType);
		InItem->AddInlineChild(NodeItem);
	}
}

void SDlgBrowser::AddEdgeNodeChildrenToItem(
	const TSharedPtr<FDlgBrowserTreeNode>& InItem,
	const TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>& EdgeNodes,
	EDlgTreeNodeTextType TextType
)
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
		const TSharedPtr<FDlgBrowserTreeNode> NodeItem =
			MakeShared<FDialogueBrowserTreeEdgeNode>(FText::FromName(Text), InItem, EdgeNode);
		NodeItem->SetTextType(TextType);
		InItem->AddInlineChild(NodeItem);
	}
}

void SDlgBrowser::AddGraphNodeBaseChildrenToItemFromProperty(
	const TSharedPtr<FDlgBrowserTreeNode>& InItem,
	const TSharedPtr<FDlgBrowserTreeVariableProperties>* PropertyPtr,
	EDlgTreeNodeTextType GraphNodeTextType,
	EDlgTreeNodeTextType EdgeNodeTextType
)
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

	const TSharedPtr<FDlgBrowserTreeVariableProperties> Property = *PropertyPtr;
	const UDlgDialogue* Dialogue = DialogueItem->GetDialogue().Get();
	const FGuid DialogueGUID = Dialogue->GetGUID();

	// Display the GraphNode
	if (Property->HasGraphNodeSet(DialogueGUID))
	{
		AddGraphNodeChildrenToItem(DialogueItem, Property->GetGraphNodeSet(DialogueGUID), GraphNodeTextType);
	}
	if (Property->HasEdgeNodeSet(DialogueGUID))
	{
		AddEdgeNodeChildrenToItem(DialogueItem, Property->GetEdgeNodeSet(DialogueGUID), EdgeNodeTextType);
	}
}

void SDlgBrowser::AddVariableChildrenToItem(
	const TSharedPtr<FDlgBrowserTreeNode>& Item,
	const TMap<FName, TSharedPtr<FDlgBrowserTreeVariableProperties>>& Variables,
	EDlgTreeNodeTextType VariableType
)
{
	for (const auto& Pair : Variables)
	{
		const FName VariableName = Pair.Key;
		const TSharedPtr<FDlgBrowserTreeNode> ChildItem =
			MakeShared<FDialogueBrowserTreeVariableNode>(FText::FromName(VariableName), Item, VariableName);
		ChildItem->SetTextType(VariableType);
		Item->AddChild(ChildItem);
	}
}

void SDlgBrowser::BuildTreeViewItem(const TSharedPtr<FDlgBrowserTreeNode>& Item)
{
	const FName ParticipantName = Item->GetParentParticipantName();
	if (!ParticipantName.IsValid() || ParticipantName.IsNone())
	{
		return;
	}

	// Do we have the Participant cached?
	TSharedPtr<FDlgBrowserTreeParticipantProperties>* ParticipantPropertiesPtr = ParticipantsProperties.Find(ParticipantName);
	if (ParticipantPropertiesPtr == nullptr)
	{
		return;
	}

	TSharedPtr<FDlgBrowserTreeParticipantProperties> ParticipantProperties = *ParticipantPropertiesPtr;
	if (Item->IsCategory())
	{
		switch (Item->GetCategoryType())
		{
		case EDlgTreeNodeCategoryType::Participant:
		{
			// Display the categories for the participant
			const bool bHideEmptyCategories = GetDefault<UDlgSystemSettings>()->bHideEmptyDialogueBrowserCategories;
			Item->SetChildren(MakeParticipantCategoriesChildren(Item, ParticipantProperties, bHideEmptyCategories));
			break;
		}
		case EDlgTreeNodeCategoryType::Dialogue:
		{
			// Add the dialogues
			for (TWeakObjectPtr<const UDlgDialogue> Dialogue : ParticipantProperties->GetDialogues())
			{
				if (!Dialogue.IsValid())
				{
					continue;
				}

				const TSharedPtr<FDlgBrowserTreeNode> DialogueItem =
					MakeShared<FDialogueBrowserTreeDialogueNode>(FText::FromName(Dialogue->GetDialogueFName()), Item, Dialogue);
				DialogueItem->SetTextType(EDlgTreeNodeTextType::ParticipantDialogue);
				Item->AddChild(DialogueItem);

			}
			break;
		}
		case EDlgTreeNodeCategoryType::Event:
		{
			// Display the events for this category
			for (const auto& Pair : ParticipantProperties->GetEvents())
			{
				const TSharedPtr<FDlgBrowserTreeNode> EventItem =
					MakeShared<FDialogueBrowserTreeVariableNode>(FText::FromName(Pair.Key), Item, Pair.Key);
				EventItem->SetTextType(EDlgTreeNodeTextType::ParticipantEvent);
				Item->AddChild(EventItem);
			}

			// Display the custom events for this category
			for (const auto& Pair : ParticipantProperties->GetCustomEvents())
			{
				UClass* Class = Pair.Key;
				const TSharedPtr<FDlgBrowserTreeNode> CustomEventItem = MakeShared<FDialogueBrowserTreeCustomObjectNode>(
					FText::FromString(FDlgHelper::CleanObjectName(Class->GetPathName())),
					Item,
					Class
				);
				CustomEventItem->SetTextType(EDlgTreeNodeTextType::ParticipantCustomEvent);
				Item->AddChild(CustomEventItem);
			}
			break;
		}
		case EDlgTreeNodeCategoryType::Condition:
		{
			for (const auto& Pair : ParticipantProperties->GetConditions())
			{
				const TSharedPtr<FDlgBrowserTreeNode> ConditionItem =
					MakeShared<FDialogueBrowserTreeVariableNode>(FText::FromName(Pair.Key), Item, Pair.Key);
				ConditionItem->SetTextType(EDlgTreeNodeTextType::ParticipantCondition);
				Item->AddChild(ConditionItem);
			}
			break;
		}

		case EDlgTreeNodeCategoryType::Variable:
		{
			// Only display the categories if the Participant has at least one variable.
			if (ParticipantProperties->HasDialogueValues())
			{
				const bool bHideEmptyCategories = GetDefault<UDlgSystemSettings>()->bHideEmptyDialogueBrowserCategories;
				Item->SetChildren(MakeVariableCategoriesChildren(Item, ParticipantProperties, bHideEmptyCategories));
			}
			break;
		}
		case EDlgTreeNodeCategoryType::VariableInt:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetIntegers(),
				EDlgTreeNodeTextType::ParticipantVariableInt
			);
			break;
		case EDlgTreeNodeCategoryType::VariableFloat:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetFloats(),
				EDlgTreeNodeTextType::ParticipantVariableFloat
			);
			break;
		case EDlgTreeNodeCategoryType::VariableBool:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetBools(),
				EDlgTreeNodeTextType::ParticipantVariableBool
			);
			break;
		case EDlgTreeNodeCategoryType::VariableFName:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetFNames(),
				EDlgTreeNodeTextType::ParticipantVariableFName
			);
			break;

		case EDlgTreeNodeCategoryType::ClassVariable:
		{
			// Only display the categories if the Participant has at least one class variable.
			if (ParticipantProperties->HasClassVariables())
			{
				const bool bHideEmptyCategories = GetDefault<UDlgSystemSettings>()->bHideEmptyDialogueBrowserCategories;
				Item->SetChildren(MakeClassVariableCategoriesChildren(Item, ParticipantProperties, bHideEmptyCategories));
			}
			break;
		}
		case EDlgTreeNodeCategoryType::ClassVariableInt:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetClassIntegers(),
				EDlgTreeNodeTextType::ParticipantClassVariableInt
			);
			break;
		case EDlgTreeNodeCategoryType::ClassVariableFloat:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetClassFloats(),
				EDlgTreeNodeTextType::ParticipantClassVariableFloat
			);
			break;
		case EDlgTreeNodeCategoryType::ClassVariableBool:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetClassBools(),
				EDlgTreeNodeTextType::ParticipantClassVariableBool
			);
			break;
		case EDlgTreeNodeCategoryType::ClassVariableFName:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetClassFNames(),
				EDlgTreeNodeTextType::ParticipantClassVariableFName
			);
			break;
		case EDlgTreeNodeCategoryType::ClassVariableFText:
			AddVariableChildrenToItem(
				Item,
				ParticipantProperties->GetClassFTexts(),
				EDlgTreeNodeTextType::ParticipantClassVariableFText
			);
			break;

		default:
			break;
		}
	}
	else if (Item->IsText())
	{
		switch (Item->GetTextType())
		{
		case EDlgTreeNodeTextType::ParticipantEvent:
			// List the dialogues that contain this event for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetEvents().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::EventDialogue
			);
			break;

		case EDlgTreeNodeTextType::ParticipantCustomEvent:
			// List the dialogues that contain this event for this custom event participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetCustomEvents().Find(Item->GetParentClass()),
				EDlgTreeNodeTextType::CustomEventDialogue
			);
			break;

		case EDlgTreeNodeTextType::ParticipantCondition:
			// List the dialogues that contain this condition for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetConditions().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::ConditionDialogue
			);
			break;

		case EDlgTreeNodeTextType::ParticipantVariableInt:
			// List the dialogues that contain this int variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetIntegers().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::IntVariableDialogue
			);
			break;
		case EDlgTreeNodeTextType::ParticipantVariableFloat:
			// List the dialogues that contain this float variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetFloats().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FloatVariableDialogue
			);
			break;
		case EDlgTreeNodeTextType::ParticipantVariableBool:
			// List the dialogues that contain this bool variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetBools().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::BoolVariableDialogue
			);
			break;
		case EDlgTreeNodeTextType::ParticipantVariableFName:
			// List the dialogues that contain this Fname variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetFNames().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FNameVariableDialogue
			);
			break;

		case EDlgTreeNodeTextType::ParticipantClassVariableInt:
			// List the dialogues that contain this UClass int variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassIntegers().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::IntClassVariableDialogue
			);
			break;
		case EDlgTreeNodeTextType::ParticipantClassVariableFloat:
			// List the dialogues that contain this UClass float variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassFloats().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FloatClassVariableDialogue
			);
			break;
		case EDlgTreeNodeTextType::ParticipantClassVariableBool:
			// List the dialogues that contain this UClass bool variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassBools().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::BoolClassVariableDialogue
			);
			break;
		case EDlgTreeNodeTextType::ParticipantClassVariableFName:
			// List the dialogues that contain this UClass Fname variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassFNames().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FNameClassVariableDialogue
			);
			break;
		case EDlgTreeNodeTextType::ParticipantClassVariableFText:
			// List the dialogues that contain this UClass FText variable for this participant
			AddDialogueChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassFTexts().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FTextClassVariableDialogue
			);
			break;

		case EDlgTreeNodeTextType::EventDialogue:
			// List the graph nodes for the dialogue that contains this event
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetEvents().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::EventGraphNode,
				EDlgTreeNodeTextType::EventGraphNode
			);
			break;

		case EDlgTreeNodeTextType::CustomEventDialogue:
			// List the graph nodes for the dialogue that contains this event
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetCustomEvents().Find(Item->GetParentClass()),
				EDlgTreeNodeTextType::CustomEventGraphNode,
				EDlgTreeNodeTextType::CustomEventGraphNode
			);
			break;

		case EDlgTreeNodeTextType::ConditionDialogue:
			// List the graph nodes for the dialogue that contains this condition
			AddGraphNodeBaseChildrenToItemFromProperty(Item,
				ParticipantProperties->GetConditions().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::ConditionGraphNode,
				EDlgTreeNodeTextType::ConditionEdgeNode);
			break;

		case EDlgTreeNodeTextType::IntVariableDialogue:
			// List the graph nodes for the dialogue that contains this int variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetIntegers().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::IntVariableGraphNode,
				EDlgTreeNodeTextType::IntVariableEdgeNode
			);
			break;
		case EDlgTreeNodeTextType::FloatVariableDialogue:
			// List the graph nodes for the dialogue that contains this float variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetFloats().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FloatVariableGraphNode,
				EDlgTreeNodeTextType::FloatVariableEdgeNode
			);
			break;
		case EDlgTreeNodeTextType::BoolVariableDialogue:
			// List the graph nodes for the dialogue that contains this bool variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetBools().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::BoolVariableGraphNode,
				EDlgTreeNodeTextType::BoolVariableEdgeNode
			);
			break;
		case EDlgTreeNodeTextType::FNameVariableDialogue:
			// List the graph nodes for the dialogue that contains this FName variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetFNames().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FNameVariableGraphNode,
				EDlgTreeNodeTextType::FNameVariableEdgeNode
			);
			break;

		case EDlgTreeNodeTextType::IntClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass int variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassIntegers().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::IntVariableGraphNode,
				EDlgTreeNodeTextType::IntVariableEdgeNode
			);
			break;
		case EDlgTreeNodeTextType::FloatClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass float variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassFloats().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FloatVariableGraphNode,
				EDlgTreeNodeTextType::FloatVariableEdgeNode
			);
			break;
		case EDlgTreeNodeTextType::BoolClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass bool variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassBools().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::BoolVariableGraphNode,
				EDlgTreeNodeTextType::BoolVariableEdgeNode
			);
			break;
		case EDlgTreeNodeTextType::FNameClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass FName variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassFNames().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FNameVariableGraphNode,
				EDlgTreeNodeTextType::FNameVariableEdgeNode
			);
			break;

		case EDlgTreeNodeTextType::FTextClassVariableDialogue:
			// List the graph nodes for the dialogue that contains this UClass FText variable
			AddGraphNodeBaseChildrenToItemFromProperty(
				Item,
				ParticipantProperties->GetClassFTexts().Find(Item->GetParentVariableName()),
				EDlgTreeNodeTextType::FTextVariableGraphNode,
				EDlgTreeNodeTextType::FTextVariableEdgeNode
			);
			break;

		default:
			break;
		}
	}

	// Recursively call on children
	for (const TSharedPtr<FDlgBrowserTreeNode>& ChildItem : Item->GetChildren())
	{
		BuildTreeViewItem(ChildItem);
	}

	// The same for the inline children, handled separately.
	for (const TSharedPtr<FDlgBrowserTreeNode>& ChildItem : Item->GetInlineChildren())
	{
		BuildTreeViewItem(ChildItem);
	}
}

TSharedRef<SWidget> SDlgBrowser::MakeButtonWidgetForGraphNodes(
	const TArray<TSharedPtr<FDlgBrowserTreeNode>>& InChildren
)
{
	TSharedPtr<SWrapBox> Buttons = SNew(SWrapBox)
			.PreferredWidth(600.f);

	// Constructs [Node 1] [Node 2]
	const FText GraphNodeTooltip = LOCTEXT("JumpToNodeTipGraphNode", "Opens the Dialogue Editor and jumps to the Node");
	const FText EdgeNodeTooltip = LOCTEXT("JumpToNodeTipEdgeNode", "Opens the Dialogue Editor and jumps to the Edge");
	for (const TSharedPtr<FDlgBrowserTreeNode>& ChildItem : InChildren)
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
					.OnClicked(ChildItem.Get(), &FDlgBrowserTreeNode::OnClick)
					[
						SNew(STextBlock)
						.Text(ChildItem->GetDisplayText())
						.Font(DEFAULT_FONT("Regular", 10))
					]
				];
		}
	}

	return Buttons.ToSharedRef();
}

TSharedRef<SWidget> SDlgBrowser::MakeInlineWidget(const TSharedPtr<FDlgBrowserTreeNode>& InItem)
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
				MakeIconAndTextWidget(InItem->GetDisplayText(), FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_DlgDialogueClassThumbnail))
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

TSharedRef<SWidget> SDlgBrowser::MakeButtonsWidgetForDialogue(const TSharedPtr<FDlgBrowserTreeNode>& InItem)
{
	return SNew(SHorizontalBox)
		// Find in Content Browser
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SButton)
			.ButtonStyle(FNYAppStyle::Get(), "HoverHintOnly")
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
					.Image(FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_FindAssetIcon))
				]
			]
		]

		// Open Editor
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SButton)
			.ButtonStyle(FNYAppStyle::Get(), "HoverHintOnly")
			.ToolTipText(LOCTEXT("OpenDialogueToolTip", "Opens the dialogue editor."))
			.OnClicked(InItem.Get(), &FDlgBrowserTreeNode::OnClick)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.WidthOverride(16)
				.HeightOverride(16)
				[
					SNew(SImage)
					.Image(FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_OpenAssetIcon))
				]
			]
		];
}

void SDlgBrowser::HandleSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	// Trim and sanitized the filter text (so that it more likely matches)
	FilterString = FText::TrimPrecedingAndTrailing(InText).ToString();
	GenerateFilteredItems();
//	RefreshTree(false);
}

TSharedRef<ITableRow> SDlgBrowser::HandleGenerateRow(
	TSharedPtr<FDlgBrowserTreeNode> InItem,
	const TSharedRef<STableViewBase>& OwnerTable
)
{
	// Build row
	TSharedPtr<STableRow<TSharedPtr<FDlgBrowserTreeNode>>> TableRow;
	FMargin RowPadding = FMargin(2.f, 2.f);
	const bool bIsCategory = InItem->IsCategory();
	const bool bIsSeparator = InItem->IsSeparator();

	if (bIsCategory)
	{
		TableRow =
			SNew(SCategoryHeaderTableRow<TSharedPtr<FDlgBrowserTreeNode>>, OwnerTable)
			.Visibility(InItem->IsVisible() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	else
	{
		TableRow =
			SNew(STableRow<TSharedPtr<FDlgBrowserTreeNode>>, OwnerTable)
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
	TSharedPtr<STextBlock> DefaultTextBlock =
		SNew(STextBlock)
		.Text(InItem->GetDisplayText())
		.HighlightText(this, &Self::GetFilterText)
		.Font(DEFAULT_FONT("Regular", 10));

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
				.Padding(FNYAppStyle::GetMargin(TEXT("Menu.Separator.Padding")))

				// Separator graphic
				.BorderImage(FNYAppStyle::GetBrush(TEXT("Menu.Separator")))
			];
	}
	else if (bIsCategory)
	{
		if (InItem->GetCategoryType() == EDlgTreeNodeCategoryType::Participant)
		{
			int32 DialogueReferences = 0;
			TSharedPtr<FDlgBrowserTreeParticipantProperties>* ParticipantPropertiesPtr =
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
					.Font(DEFAULT_FONT("Regular", 16))
					.Text(InItem->GetDisplayText())
					.HighlightText(this, &Self::GetFilterText)
				]

				// Number of dialogue references
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(DEFAULT_FONT("Regular", 9))
					.Text(FText::FromString(FString::Printf(TEXT("Dialogue references %d"), DialogueReferences)))
				];
		}
		else
		{
			RowContent = SNew(STextBlock)
				.Font(DEFAULT_FONT("Regular", 12))
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
						FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_DlgDialogueClassThumbnail))
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
			RowContent = MakeIconAndTextWidget(
				InItem->GetDisplayText(),
				FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_EventIcon)
			);
		}
		else if (InItem->IsCustomEventText())
		{
			RowContent = MakeCustomObjectIconAndTextWidget(
				InItem->GetDisplayText(),
				FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_EventIcon),
				InItem->GetParentClass(),
				EDlgBlueprintOpenType::Event,
				GET_FUNCTION_NAME_CHECKED(UDlgEventCustom, EnterEvent)
			);
		}
		else if (InItem->IsConditionText())
		{
			RowContent = MakeIconAndTextWidget(
				InItem->GetDisplayText(),
				FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_ConditionIcon)
			);
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
					.OnClicked(InItem.Get(), &FDlgBrowserTreeNode::OnClick)
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

void SDlgBrowser::HandleGetChildren(TSharedPtr<FDlgBrowserTreeNode> InItem, TArray<TSharedPtr<FDlgBrowserTreeNode>>& OutChildren)
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

void SDlgBrowser::HandleTreeSelectionChanged(TSharedPtr<FDlgBrowserTreeNode> NewValue, ESelectInfo::Type SelectInfo)
{
	// Ignore
}

void SDlgBrowser::HandleDoubleClick(TSharedPtr<FDlgBrowserTreeNode> InItem)
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

void SDlgBrowser::HandleSetExpansionRecursive(TSharedPtr<FDlgBrowserTreeNode> InItem, bool bInIsItemExpanded)
{
	if (InItem.IsValid() && InItem->HasChildren())
	{
		ParticipantsTreeView->SetItemExpansion(InItem, bInIsItemExpanded);
		for (const TSharedPtr<FDlgBrowserTreeNode>& Child : InItem->GetChildren())
		{
			HandleSetExpansionRecursive(Child, bInIsItemExpanded);
		}
	}
}

void SDlgBrowser::HandleSortSelectionChanged(SortOptionType Selection, ESelectInfo::Type SelectInfo)
{
	if (Selection.IsValid())
	{
		SelectedSortOption = Selection;
		RefreshTree(true);
	}
}

FReply SDlgBrowser::FindInContentBrowserForItem(TSharedPtr<FDlgBrowserTreeNode> InItem)
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

TArray<TSharedPtr<FDlgBrowserTreeNode>> SDlgBrowser::MakeParticipantCategoriesChildren(
	const TSharedPtr<FDlgBrowserTreeNode>& Parent,
	const TSharedPtr<FDlgBrowserTreeParticipantProperties>& ParticipantProperties,
	bool bHideEmptyCategories
) const
{
	TArray<TSharedPtr<FDlgBrowserTreeNode>> Categories;
	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasDialogues()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Dialogues")),
			Parent,
			EDlgTreeNodeCategoryType::Dialogue
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasEvents()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Events")),
			Parent,
			EDlgTreeNodeCategoryType::Event
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasConditions()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Conditions")),
			Parent,
			EDlgTreeNodeCategoryType::Condition
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasDialogueValues()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Dialogue Values")),
			Parent,
			EDlgTreeNodeCategoryType::Variable
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassVariables()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Class Variables")),
			Parent,
			EDlgTreeNodeCategoryType::ClassVariable
		);
		Categories.Add(Category);
	}
	return Categories;
}

TArray<TSharedPtr<FDlgBrowserTreeNode>> SDlgBrowser::MakeVariableCategoriesChildren(
	const TSharedPtr<FDlgBrowserTreeNode>& Parent,
	const TSharedPtr<FDlgBrowserTreeParticipantProperties>& ParticipantProperties,
	bool bHideEmptyCategories
) const
{
	TArray<TSharedPtr<FDlgBrowserTreeNode>> Categories;
	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasIntegers()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Integers")),
			Parent,
			EDlgTreeNodeCategoryType::VariableInt
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasFloats()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Floats")),
			Parent,
			EDlgTreeNodeCategoryType::VariableFloat
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasBools()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Bools")),
			Parent,
			EDlgTreeNodeCategoryType::VariableBool
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasFNames()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("FNames")),
			Parent,
			EDlgTreeNodeCategoryType::VariableFName
		);
		Categories.Add(Category);
	}

	return Categories;
}

TArray<TSharedPtr<FDlgBrowserTreeNode>> SDlgBrowser::MakeClassVariableCategoriesChildren(
	const TSharedPtr<FDlgBrowserTreeNode>& Parent,
	const TSharedPtr<FDlgBrowserTreeParticipantProperties>& ParticipantProperties,
	bool bHideEmptyCategories
) const
{
	TArray<TSharedPtr<FDlgBrowserTreeNode>> Categories;

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassIntegers()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Integers")),
			Parent,
			EDlgTreeNodeCategoryType::ClassVariableInt
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassFloats()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Floats")),
			Parent,
			EDlgTreeNodeCategoryType::ClassVariableFloat
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassBools()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("Bools")),
			Parent,
			EDlgTreeNodeCategoryType::ClassVariableBool
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassFNames()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("FNames")),
			Parent,
			EDlgTreeNodeCategoryType::ClassVariableFName
		);
		Categories.Add(Category);
	}

	if (!bHideEmptyCategories || (bHideEmptyCategories && ParticipantProperties->HasClassFTexts()))
	{
		TSharedPtr<FDlgBrowserTreeNode> Category = MakeShared<FDialogueBrowserTreeCategoryNode>(
			FText::FromString(TEXT("FTexts")),
			Parent,
			EDlgTreeNodeCategoryType::ClassVariableFText
		);
		Categories.Add(Category);
	}

	return Categories;
}

TSharedRef<SHorizontalBox> SDlgBrowser::MakeIconAndTextWidget(
	const FText& InText,
	const FSlateBrush* IconBrush,
	int32 IconSize
)
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

TSharedRef<SHorizontalBox> SDlgBrowser::MakeCustomObjectIconAndTextWidget(
	const FText& InText,
	const FSlateBrush* IconBrush,
	UClass* Class,
	EDlgBlueprintOpenType OpenType,
	FName FunctionNameToOpen,
	int32 IconSize
)
{
	TSharedRef<SHorizontalBox> HorizontalBox = MakeIconAndTextWidget(InText, IconBrush, IconSize);

	// Browse Asset
	HorizontalBox->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(4.f)
	[
		SNew(SButton)
		.ButtonStyle(FNYAppStyle::Get(), "HoverHintOnly")
		.ToolTipText_Static(&Self::GetBrowseAssetText, Class)
		.ContentPadding(4.f)
		.ForegroundColor(FSlateColor::UseForeground())
		.Visibility_Static(&Self::GetBrowseAssetButtonVisibility, Class)
		.OnClicked_Static(&Self::OnBrowseAssetClicked, Class)
		[
			SNew(SImage)
			.Image(FNYAppStyle::GetBrush("PropertyWindow.Button_Browse"))
			.ColorAndOpacity(FSlateColor::UseForeground())
		]
	];

	// Jump to Object
	HorizontalBox->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(4.f, 2.f)
	[
		SNew(SButton)
		.ButtonStyle(FNYAppStyle::Get(), "HoverHintOnly")
		.ToolTipText_Static(&Self::GetJumpToAssetText, Class)
		.ContentPadding(4.f)
		.ForegroundColor(FSlateColor::UseForeground())
		.Visibility_Static(&Self::GetOpenAssetButtonVisibility, Class)
		.OnClicked_Static(&Self::OnOpenAssetClicked, Class, OpenType, FunctionNameToOpen)
		[
			SNew(SImage)
			 .Image(FNYAppStyle::GetBrush("PropertyWindow.Button_Edit"))
			 .ColorAndOpacity( FSlateColor::UseForeground() )
		]
	];

	return HorizontalBox;
}

EVisibility SDlgBrowser::GetOpenAssetButtonVisibility(UClass* Class)
{
	// Blueprint, always visible
	if (FDlgHelper::IsABlueprintClass(Class))
	{
		return EVisibility::Visible;
	}

	// Native
	return FSourceCodeNavigation::CanNavigateToClass(Class) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SDlgBrowser::GetBrowseAssetButtonVisibility(UClass* Class)
{
	// Blueprint, always Visible
	if (FDlgHelper::IsABlueprintClass(Class))
	{
		return EVisibility::Visible;
	}

	// Native Hide
	return EVisibility::Collapsed;
}

FReply SDlgBrowser::OnBrowseAssetClicked(UClass* Class)
{
	UBlueprint* Blueprint = nullptr;
	if (const UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(Class))
	{
		Blueprint = Cast<UBlueprint>(BlueprintClass->ClassGeneratedBy);
	}

	static constexpr bool bFocusContentBrowser = true;
	TArray<UObject*> ObjectsToSyncTo;
	if (Blueprint)
	{
		ObjectsToSyncTo.Add(Blueprint);
	}
	GEditor->SyncBrowserToObjects(ObjectsToSyncTo, bFocusContentBrowser);

	return FReply::Handled();
}

FReply SDlgBrowser::OnOpenAssetClicked(
	UClass* Class,
	EDlgBlueprintOpenType OpenType,
	FName FunctionNameToOpen
)
{
	UBlueprint* Blueprint = nullptr;
	if (const UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(Class))
	{
		Blueprint = Cast<UBlueprint>(BlueprintClass->ClassGeneratedBy);
	}

	if (Blueprint)
	{
		static constexpr bool bForceFullEditor = true;
		static constexpr bool bAddBlueprintFunctionIfItDoesNotExist = true;
		FDlgEditorUtilities::OpenBlueprintEditor(
			Blueprint,
			OpenType,
			FunctionNameToOpen,
			bForceFullEditor,
			bAddBlueprintFunctionIfItDoesNotExist
		);
	}
	else if (UObject* Object = Class->GetDefaultObject())
	{
		// Native
		FSourceCodeNavigation::NavigateToClass(Object->GetClass());
	}

	return FReply::Handled();
}

FText SDlgBrowser::GetJumpToAssetText(UClass* Class)
{
	// Blueprint, always visible
	if (FDlgHelper::IsABlueprintClass(Class))
	{
		return LOCTEXT("OpenObjectBlueprintTooltipKey", "Open Blueprint Editor");
	}

	// Native
	return FText::Format(
		LOCTEXT("OpenObjectBlueprintTooltipKey", "Open Source File in {0}"),
		FSourceCodeNavigation::GetSelectedSourceCodeIDE()
	);
}

FText SDlgBrowser::GetBrowseAssetText(UClass* Class)
{
	return LOCTEXT("BrowseButtonToolTipText", "Browse to Asset in Content Browser");
}

TSharedRef<SWidget> SDlgBrowser::FillViewOptionsEntries()
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
#undef DEFAULT_FONT
