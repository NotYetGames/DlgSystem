// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEditor.h"

#include "Editor.h"
#include "SSingleObjectDetailsPanel.h"
#include "Widgets/Docking/SDockTab.h"
#include "ScopedTransaction.h"
#include "Framework/Application/SlateApplication.h"
#include "PropertyEditorModule.h"
#include "GraphEditor.h"
#include "Toolkits/IToolkitHost.h"
#include "IDetailsView.h"
#include "Framework/Commands/GenericCommands.h"
#include "GraphEditorActions.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EdGraphUtilities.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Editor/Transactor.h"

#include "DlgSystemEditor/DlgSystemEditorModule.h"
#include "DlgSystem/DlgDialogue.h"
#include "DlgSystem/DlgHelper.h"
#include "SDlgEditorPalette.h"
#include "SDlgDialogueActionMenu.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Root.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgSystemEditor/Editor/Graph/DialogueGraphSchema.h"
#include "DlgSystemEditor/DlgCommands.h"
#include "Graph/SchemaActions/DlgConvertSpeechSequenceNodeToSpeechNodes_GraphSchemaAction.h"
#include "DlgSystemEditor/Search/DlgSearchManager.h"
#include "DlgSystemEditor/Search/SDlgFindInDialogues.h"
#include "Graph/SchemaActions/DlgConvertSpeechNodesToSpeechSequence_GraphSchemaAction.h"

#define LOCTEXT_NAMESPACE "DialogueEditor"

// define constants
const FName FDlgEditor::DetailsTabID(TEXT("DialogueEditor_Details"));
const FName FDlgEditor::GraphCanvasTabID(TEXT("DialogueEditor_GraphCanvas"));
const FName FDlgEditor::PaletteTabId(TEXT("DialogueEditor_Palette"));
const FName FDlgEditor::FindInDialogueTabId(TEXT("DialogueEditor_Find"));
const FName DialogueEditorAppName = FName(TEXT("DialogueEditorAppName"));

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgEditor
FDlgEditor::FDlgEditor() : DialogueBeingEdited(nullptr)
{
	GEditor->RegisterForUndo(this);
}

FDlgEditor::~FDlgEditor()
{
	GEditor->UnregisterForUndo(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin IToolkit interface
void FDlgEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_DialogueEditor", "Dialogue Editor"));
	const TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// spawn tabs
	InTabManager->RegisterTabSpawner(
		GraphCanvasTabID,
		FOnSpawnTab::CreateSP(this, &Self::SpawnTab_GraphCanvas)
	)
	.SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
	.SetGroup(WorkspaceMenuCategoryRef)
	.SetIcon(FSlateIcon(NY_GET_APP_STYLE_NAME(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(
		DetailsTabID,
		FOnSpawnTab::CreateSP(this, &Self::SpawnTab_Details)
	)
	.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
	.SetGroup(WorkspaceMenuCategoryRef)
	.SetIcon(FSlateIcon(NY_GET_APP_STYLE_NAME(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(
		PaletteTabId,
		FOnSpawnTab::CreateSP(this, &Self::SpawnTab_Palette)
	)
	.SetDisplayName(LOCTEXT("PaletteTab", "Palette"))
	.SetGroup(WorkspaceMenuCategoryRef)
	.SetIcon(FSlateIcon(NY_GET_APP_STYLE_NAME(), "Kismet.Tabs.Palette"));

	InTabManager->RegisterTabSpawner(
		FindInDialogueTabId,
		FOnSpawnTab::CreateSP(this, &Self::SpawnTab_FindInDialogue)
	)
	.SetDisplayName(LOCTEXT("FindInDialogueTab", "Find Results"))
	.SetGroup(WorkspaceMenuCategoryRef)
	.SetIcon(FSlateIcon(NY_GET_APP_STYLE_NAME(), "Kismet.Tabs.FindResults"));
}

void FDlgEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(GraphCanvasTabID);
	InTabManager->UnregisterTabSpawner(DetailsTabID);
	InTabManager->UnregisterTabSpawner(PaletteTabId);
	InTabManager->UnregisterTabSpawner(FindInDialogueTabId);
}

FText FDlgEditor::GetBaseToolkitName() const
{
	return LOCTEXT("DialogueEditorAppLabel", "Dialogue Editor");
}

FText FDlgEditor::GetToolkitName() const
{
	const bool bDirtyState = DialogueBeingEdited->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("DialogueName"), FText::FromString(DialogueBeingEdited->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("DialogueEditorToolkitName", "{DialogueName}{DirtyState}"), Args);
}
// End of IToolkit Interface
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin FAssetEditorToolkit
void FDlgEditor::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();
}

void FDlgEditor::SaveAssetAs_Execute()
{
	FAssetEditorToolkit::SaveAssetAs_Execute();
}
// End of FAssetEditorToolkit
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin IAssetEditorInstance
void FDlgEditor::FocusWindow(UObject* ObjectToFocusOn)
{
	BringToolkitToFront();
	JumpToObject(ObjectToFocusOn);
}
// End of IAssetEditorInstance
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin FEditorUndoClient Interface
void FDlgEditor::PostUndo(bool bSuccess)
{
	if (bSuccess)
	{
		Refresh(false);
		CheckAll();
	}
}

void FDlgEditor::PostRedo(bool bSuccess)
{
	if (bSuccess)
	{
		Refresh(false);
		CheckAll();
	}
}
// End of FEditorUndoClient Interface
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin IDlgEditor

void FDlgEditor::RefreshDetailsView(bool bRestorePreviousSelection)
{
	if (DetailsView.IsValid())
	{
		if (DialogueBeingEdited)
		{
			DetailsView->SetObject(DialogueBeingEdited, true);
		}
		else
		{
			DetailsView->ForceRefresh();
		}
		DetailsView->ClearSearch();
	}

	if (bRestorePreviousSelection)
	{
		// Create local copy because this can be changed by node selection again
		TArray<TWeakObjectPtr<UObject>> ArrayCopy = PreviousSelectedNodeObjects;

		// Select all previous nodes
		for (const TWeakObjectPtr<UObject>& WeakObj : ArrayCopy)
		{
			if (!WeakObj.IsValid(false))
			{
				continue;
			}

			UObject* Object = WeakObj.Get();
			if (!IsValid(Object))
			{
				continue;
			}

			UEdGraphNode* GraphNode = Cast<UEdGraphNode>(Object);
			if (!IsValid(GraphNode))
			{
				continue;
			}

			GraphEditorView->SetNodeSelection(const_cast<UEdGraphNode*>(GraphNode), true);
		}
	}
}

void FDlgEditor::Refresh(bool bRestorePreviousSelection)
{
	ClearViewportSelection();
	RefreshViewport();
	RefreshDetailsView(bRestorePreviousSelection);
	FSlateApplication::Get().DismissAllMenus();
}

void FDlgEditor::JumpToObject(const UObject* Object)
{
	// Ignore invalid objects
	if (!IsValid(Object) || !GraphEditorView.IsValid())
	{
		return;
	}

	const UEdGraphNode* GraphNode = Cast<UEdGraphNode>(Object);
	if (!IsValid(GraphNode))
	{
		return;
	}

	// Are we in the same graph?
	if (DialogueBeingEdited->GetGraph() != GraphNode->GetGraph())
	{
		return;
	}

	// TODO create custom SGraphEditor
	// Not part of the graph anymore :(
	if (!DialogueBeingEdited->GetGraph()->Nodes.Contains(GraphNode))
	{
		return;
	}

	// Jump to the node
	static constexpr bool bRequestRename = false;
	static constexpr bool bSelectNode = true;
	Refresh(false);
	GraphEditorView->JumpToNode(GraphNode, bRequestRename, bSelectNode);

	// Select from JumpNode seems to be buggy sometimes, WE WILL DO IT OURSELFS!
	// I know, I know, it is not my fault that SetNodeSelection does not have the graph node as const, sigh
	GraphEditorView->SetNodeSelection(const_cast<UEdGraphNode*>(GraphNode), bSelectNode);
}
// End of IDlgEditor
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own functions
void FDlgEditor::SummonSearchUI(bool bSetFindWithinDialogue, FString NewSearchTerms, bool bSelectFirstResult)
{
	TSharedPtr<SDlgFindInDialogues> FindResultsToUse;
	if (bSetFindWithinDialogue)
	{
		// Open local tab
		FindResultsToUse = FindResultsView;
		FDlgHelper::InvokeTab(TabManager, FindInDialogueTabId);
	}
	else
	{
		// Open global tab
		FindResultsToUse = FDlgSearchManager::Get()->GetGlobalFindResults();
	}

	if (FindResultsToUse.IsValid())
	{
		FDlgSearchFilter Filter;
		Filter.SearchString = NewSearchTerms;
		FindResultsToUse->FocusForUse(bSetFindWithinDialogue, Filter, bSelectFirstResult);
	}
}

void FDlgEditor::InitDialogueEditor(
	EToolkitMode::Type Mode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost,
	UDlgDialogue* InitDialogue
)
{
	Settings = GetMutableDefault<UDlgSystemSettings>();

	// close all other editors editing this asset
	FDlgEditorUtilities::CloseOtherEditors(InitDialogue, this);
	DialogueBeingEdited = InitDialogue;
	FDlgEditorUtilities::TryToCreateDefaultGraph(DialogueBeingEdited);

	// Bind Undo/Redo methods
	DialogueBeingEdited->SetFlags(RF_Transactional);

	// Gather data for the showing of primary/secondary edges
	if (GetSettings().bShowPrimarySecondaryEdges)
	{
		DialogueBeingEdited->InitialCompileDialogueNodesFromGraphNodes();
	}

	// Bind commands
	FGraphEditorCommands::Register();
	FDlgCommands::Register();
	BindEditorCommands();
	CreateInternalWidgets();

	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout =
		FTabManager::NewLayout("Standalone_DialogueEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				// Toolbar
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
#if NY_ENGINE_VERSION < 500
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
#endif
				->SetHideTabWell(true)
			)
			->Split
			(
				// Main Application area
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					// Left
					// Details tab
					FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->AddTab(DetailsTabID, ETabState::OpenedTab)
				)
				->Split
				(
					// Middle
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.6f)
					->Split
					(
						// Top
						// Graph canvas
						FTabManager::NewStack()
						->SetSizeCoefficient(0.8f)
						->SetHideTabWell(true)
						->AddTab(GraphCanvasTabID, ETabState::OpenedTab)
					)
					->Split
					(
						// Bottom
						// Find Dialogue results
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(FindInDialogueTabId, ETabState::ClosedTab)
					)

				)
				->Split
				(
					// Right
					// Properties tab
					FTabManager::NewStack()
					->SetSizeCoefficient(0.1f)
					->AddTab(PaletteTabId, ETabState::OpenedTab)
				)

			)
		);

	// Initialize the asset editor and spawn nothing (dummy layout)
	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;
	constexpr bool bInIsToolbarFocusable = false;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, DialogueEditorAppName, StandaloneDefaultLayout,
			bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, /*ObjectToEdit =*/ InitDialogue, bInIsToolbarFocusable);

	// extend menus and toolbar
	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

	// Default show all nodes on editor open
	OnCommandUnHideAllNodes();
}

void FDlgEditor::CheckAll() const
{
#if DO_CHECK
	check(DialogueBeingEdited);
	UDialogueGraph* Graph = GetDialogueGraph();
	for (UEdGraphNode* GraphNode : Graph->Nodes)
	{
		check(GraphNode);
	}

	check(FDlgEditorUtilities::AreDialogueNodesInSyncWithGraphNodes(DialogueBeingEdited));
	for (UDialogueGraphNode* GraphNode : Graph->GetAllDialogueGraphNodes())
	{
		GraphNode->CheckAll();
	}
#endif
}

void FDlgEditor::SetDialogueBeingEdited(UDlgDialogue* NewDialogue)
{
	// TODO do we need this method?

	// not different or a null poointer, do not set anything
	if (NewDialogue == DialogueBeingEdited || !IsValid(NewDialogue))
		return;

	// set to the new dialogue
	UDlgDialogue* OldDialogue = DialogueBeingEdited;
	DialogueBeingEdited = NewDialogue;

	// Let the viewport know that we are editing something different

	// Let the editor know that are editing something different
	RemoveEditingObject(OldDialogue);
	AddEditingObject(NewDialogue);

	// Update the asset picker to select the new active dialogue
}

void FDlgEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	if (GraphEditorView.IsValid() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		GraphEditorView->NotifyGraphChanged();
	}
}

void FDlgEditor::CreateInternalWidgets()
{
	// The graph Viewport
	GraphEditorView = CreateGraphEditorWidget();

	// Details View (properties panel)
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	DetailsViewArgs.bHideSelectionTip = false;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(DialogueBeingEdited);

	// Pallete view
	PaletteView = SNew(SDlgEditorPalette);

	// Find Results
	FindResultsView = SNew(SDlgFindInDialogues, SharedThis(this));
}

TSharedRef<SGraphEditor> FDlgEditor::CreateGraphEditorWidget()
{
	check(DialogueBeingEdited);
	// Customize the appearance of the graph.
	FGraphAppearanceInfo AppearanceInfo;
	// The text that appears on the bottom right corner in the graph view.
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_DlgDialogue", "DIALOGUE");
	AppearanceInfo.InstructionText = LOCTEXT("AppearanceInstructionText_DlgDialogue", "Right Click to add new nodes.");

	// Bind graph events actions from the editor
	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FDlgEditor::OnNodeTitleCommitted);
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FDlgEditor::OnSelectedNodesChanged);
	InEvents.OnCreateActionMenu = SGraphEditor::FOnCreateActionMenu::CreateSP(this, &FDlgEditor::OnCreateGraphActionMenu);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(DialogueBeingEdited->GetGraph())
		.GraphEvents(InEvents)
		.ShowGraphStateOverlay(false);
}

void FDlgEditor::BindEditorCommands()
{
	// Prevent duplicate assigns. This should never happen
	if (GraphEditorCommands.IsValid())
	{
		return;
	}
	GraphEditorCommands = MakeShared<FUICommandList>();

	// Graph Editor Commands
	// Create comment node on graph. Default when you press the "C" key on the keyboard to create a comment.
	GraphEditorCommands->MapAction(
		FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateLambda([this]
		{
			FDlgNewComment_GraphSchemaAction CommentAction;
			CommentAction.PerformAction(DialogueBeingEdited->GetGraph(), nullptr, GraphEditorView->GetPasteLocation());
		})
	);

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateLambda([this] { GraphEditorView->SelectAllNodes(); } ),
		FCanExecuteAction::CreateLambda([] { return true; })
	);

	// Edit Node commands
	const auto DialogueCommands = FDlgCommands::Get();
	GraphEditorCommands->MapAction(
		DialogueCommands.ConvertSpeechSequenceNodeToSpeechNodes,
		FExecuteAction::CreateSP(this, &Self::OnCommandConvertSpeechSequenceNodeToSpeechNodes),
		FCanExecuteAction::CreateLambda([this]
		{
			return FDlgEditorUtilities::CanConvertSpeechSequenceNodeToSpeechNodes(GetSelectedNodes());
		})
	);

	GraphEditorCommands->MapAction(
		DialogueCommands.ConvertSpeechNodesToSpeechSequence,
		FExecuteAction::CreateSP(this, &Self::OnCommandConvertSpeechNodesToSpeechSequence),
		FCanExecuteAction::CreateLambda([this]
		{
			return FDlgEditorUtilities::CanConvertSpeechNodesToSpeechSequence(GetSelectedNodes());
		})
	);

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &Self::OnCommandDeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &Self::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &Self::OnCommandCopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &Self::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &Self::OnCommandPasteNodes),
		FCanExecuteAction::CreateRaw(this, &Self::CanPasteNodes)
	);

	// Hide/Unhide nodes
	GraphEditorCommands->MapAction(
		DialogueCommands.HideNodes,
		FExecuteAction::CreateRaw(this, &Self::OnCommandHideSelectedNodes)
	);

	GraphEditorCommands->MapAction(
		DialogueCommands.UnHideAllNodes,
		FExecuteAction::CreateRaw(this, &Self::OnCommandUnHideAllNodes)
	);

	// Toolikit/Toolbar commands/Menu Commands
	// Undo Redo menu options
	ToolkitCommands->MapAction(
		FGenericCommands::Get().Undo,
		FExecuteAction::CreateSP(this, &Self::OnCommandUndoGraphAction)
	);

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Redo,
		FExecuteAction::CreateSP(this, &Self::OnCommandRedoGraphAction)
	);

	// The toolbar reload button
	ToolkitCommands->MapAction(
		DialogueCommands.DialogueReloadData,
		FExecuteAction::CreateSP(this, &Self::OnCommandDialogueReload),
		FCanExecuteAction::CreateLambda([this]
		{
			return GetSettings().DialogueTextFormat != EDlgDialogueTextFormat::None;
		})
	);

	// The Show primary/secondary edge buttons
	ToolkitCommands->MapAction(
		DialogueCommands.ToggleShowPrimarySecondaryEdges,
		FExecuteAction::CreateLambda([this]
		{
			Settings->SetShowPrimarySecondaryEdges(!GetSettings().bShowPrimarySecondaryEdges);
			if (GetSettings().bShowPrimarySecondaryEdges)
			{
				DialogueBeingEdited->InitialCompileDialogueNodesFromGraphNodes();
			}
		}),
		FCanExecuteAction::CreateLambda([] { return true; } ),
		FIsActionChecked::CreateLambda([this] { return GetSettings().bShowPrimarySecondaryEdges; })
	);

	ToolkitCommands->MapAction(
		DialogueCommands.ToggleDrawPrimaryEdges,
		FExecuteAction::CreateLambda([this] { Settings->SetDrawPrimaryEdges(!GetSettings().bDrawPrimaryEdges); }),
		FCanExecuteAction::CreateLambda([this] { return GetSettings().bShowPrimarySecondaryEdges; }),
		FIsActionChecked::CreateLambda([this] { return GetSettings().bDrawPrimaryEdges; })
	);

	ToolkitCommands->MapAction(
		DialogueCommands.ToggleDrawSecondaryEdges,
		FExecuteAction::CreateLambda([this] { Settings->SetDrawSecondaryEdges(!GetSettings().bDrawSecondaryEdges); }),
		FCanExecuteAction::CreateLambda([this] { return GetSettings().bShowPrimarySecondaryEdges; }),
		FIsActionChecked::CreateLambda([this] { return GetSettings().bDrawSecondaryEdges; })
	);

	// Find in All Dialogues
	ToolkitCommands->MapAction(
		FDlgCommands::Get().FindInAllDialogues,
		FExecuteAction::CreateLambda([this] { SummonSearchUI(false); })
	);

	// Find in current Dialogue
	ToolkitCommands->MapAction(
		FDlgCommands::Get().FindInDialogue,
		FExecuteAction::CreateLambda([this] { SummonSearchUI(true); })
	);

	// Map the global actions
	FDlgSystemEditorModule::MapActionsForFileMenuExtender(ToolkitCommands);
	FDlgSystemEditorModule::MapActionsForHelpMenuExtender(ToolkitCommands);

	ToolkitCommands->MapAction(
		FDlgCommands::Get().DeleteCurrentDialogueTextFiles,
		FExecuteAction::CreateLambda([this]
		{
			check(DialogueBeingEdited);
			const FString Text = TEXT("Delete All text files for this Dialogue?");
			const EAppReturnType::Type Response = FPlatformMisc::MessageBoxExt(EAppMsgType::YesNo, *Text, *Text);
			if (Response == EAppReturnType::Yes)
			{
				DialogueBeingEdited->DeleteAllTextFiles();
			}
		})
	);
}

void FDlgEditor::ExtendMenu()
{
	TSharedPtr<FExtender> MenuExtender = MakeShared<FExtender>();

	// Extend the Edit menu
	MenuExtender->AddMenuExtension(
		"EditHistory",
		EExtensionHook::After,
		ToolkitCommands,
		FMenuExtensionDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.BeginSection("EditSearch", LOCTEXT("EditMenu_SearchHeading", "Search") );
			{
				MenuBuilder.AddMenuEntry(FDlgCommands::Get().FindInDialogue);
				MenuBuilder.AddMenuEntry(FDlgCommands::Get().FindInAllDialogues);
			}
			MenuBuilder.EndSection();
		})
	);

	AddMenuExtender(MenuExtender);

	AddMenuExtender(
		FDlgSystemEditorModule::CreateFileMenuExtender(
			ToolkitCommands,
			{FDlgCommands::Get().DeleteCurrentDialogueTextFiles}
		)
	);
	AddMenuExtender(FDlgSystemEditorModule::CreateHelpMenuExtender(ToolkitCommands));
}

TSharedRef<SWidget> FDlgEditor::GeneratePrimarySecondaryEdgesMenu() const
{
	constexpr bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, ToolkitCommands, {});
	MenuBuilder.BeginSection("ViewingOptions", LOCTEXT("PrimarySecondaryEdgesMenu", "Viewing Options"));
	{
		MenuBuilder.AddMenuEntry(FDlgCommands::Get().ToggleDrawPrimaryEdges);
		MenuBuilder.AddMenuEntry(FDlgCommands::Get().ToggleDrawSecondaryEdges);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> FDlgEditor::GenerateExternalURLsMenu() const
{
	constexpr bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, ToolkitCommands, {});
	MenuBuilder.BeginSection("OtherURls", LOCTEXT("OtherURLsMenu", "Other URLs"));
	{
		MenuBuilder.AddMenuEntry(FDlgCommands::Get().OpenMarketplace);
		MenuBuilder.AddMenuEntry(FDlgCommands::Get().OpenWiki);
		MenuBuilder.AddMenuEntry(FDlgCommands::Get().OpenDiscord);
		MenuBuilder.AddMenuEntry(FDlgCommands::Get().OpenForum);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FDlgEditor::ExtendToolbar()
{
	// Make toolbar to the right of the Asset Toolbar (Save and Find in content browser buttons).
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
		ToolbarExtender->AddToolBarExtension(
			"Asset",
			EExtensionHook::After,
			ToolkitCommands,
			FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
			{
				ToolbarBuilder.BeginSection("Dialogue");
				{
					ToolbarBuilder.AddToolBarButton(FDlgCommands::Get().DialogueReloadData);
					ToolbarBuilder.AddToolBarButton(FDlgCommands::Get().FindInDialogue);

					ToolbarBuilder.AddToolBarButton(FDlgCommands::Get().ToggleShowPrimarySecondaryEdges);
					ToolbarBuilder.AddComboButton(
						FUIAction(
							FExecuteAction(),
							FCanExecuteAction::CreateLambda([this] {
								// only when the show/primary secondary edges is true
								return GetSettings().bShowPrimarySecondaryEdges;
							})
						),
						FOnGetContent::CreateSP(this, &Self::GeneratePrimarySecondaryEdgesMenu),
						LOCTEXT("PrimarySecondaryEdges", "Viewing Options"),
						LOCTEXT("PrimarySecondaryEdges_ToolTip", "Viewing settings"),
						FSlateIcon(),
						true
					);
				}
				ToolbarBuilder.EndSection();
			})
		);
		AddToolbarExtender(ToolbarExtender);
	}

	// Add External URLs
	if (GetSettings().bShowExternalURLsToolbar)
	{
		TSharedPtr<FExtender> ExternalToolbarExtender = MakeShared<FExtender>();
		ExternalToolbarExtender->AddToolBarExtension(
			"Asset",
			EExtensionHook::After,
			ToolkitCommands,
			FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
			{
				ToolbarBuilder.BeginSection("DialogueExternal");
				{
					ToolbarBuilder.AddToolBarButton(
						FDlgCommands::Get().OpenNotYetPlugins,
						NAME_None,
						LOCTEXT("NotYetPlugins", "Not Yet: Plugins"),
						{},
						FSlateIcon(FDlgStyle::GetStyleSetName(), FDlgStyle::PROPERTY_NotYetLogoIcon)
					);
					ToolbarBuilder.AddComboButton(
						FUIAction(),
						FOnGetContent::CreateSP(this, &Self::GenerateExternalURLsMenu),
						LOCTEXT("OtherURLs", "Other URLs"),
						LOCTEXT("OtherURLs_ToolTip", "Other URLs"),
						FSlateIcon(),
						true
					);
				}
				ToolbarBuilder.EndSection();
			})
		);
		AddToolbarExtender(ExternalToolbarExtender);
	}
}

TSharedRef<SDockTab> FDlgEditor::SpawnTab_Details(const FSpawnTabArgs& Args) const
{
	check(Args.GetTabId() == DetailsTabID);

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.Label(LOCTEXT("DialogueDetailsTitle", "Details"))
		.TabColorScale(GetTabColorScale())
		[
			DetailsView.ToSharedRef()
		];

	// TODO use DialogueEditor.Tabs.Properties
	const auto* IconBrush = FNYAppStyle::GetBrush(TEXT("GenericEditor.Tabs.Properties"));
	NewTab->SetTabIcon(IconBrush);

	return NewTab;
}

TSharedRef<SDockTab> FDlgEditor::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args) const
{
	check(Args.GetTabId() == GraphCanvasTabID);
	return SNew(SDockTab)
		.Label(LOCTEXT("DialogueGraphCanvasTiele", "Viewport"))
		[
			GraphEditorView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FDlgEditor::SpawnTab_Palette(const FSpawnTabArgs& Args) const
{
	check(Args.GetTabId() == PaletteTabId);

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.Label(LOCTEXT("DialoguePaletteTitle", "Palette"))
		[
			PaletteView.ToSharedRef()
		];

	const auto* IconBrush = FNYAppStyle::GetBrush(TEXT("Kismet.Tabs.Palette"));
	NewTab->SetTabIcon(IconBrush);

	return NewTab;
}

TSharedRef<SDockTab> FDlgEditor::SpawnTab_FindInDialogue(const FSpawnTabArgs& Args) const
{
	check(Args.GetTabId() == FindInDialogueTabId);

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.Label(LOCTEXT("FindResultsView", "Find Results"))
		[
			FindResultsView.ToSharedRef()
		];

	const auto* IconBrush = FNYAppStyle::GetBrush(TEXT("Kismet.Tabs.FindResults"));
	NewTab->SetTabIcon(IconBrush);

	return NewTab;
}

void FDlgEditor::OnCommandUndoGraphAction() const
{
	constexpr bool bCanRedo = true;
	if (!GEditor->UndoTransaction(bCanRedo))
	{
		const FText TransactionName = GEditor->Trans->GetUndoContext(true).Title;
		UE_LOG(LogDlgSystemEditor, Warning, TEXT("Undo Transaction with Name = `%s` failed"), *TransactionName.ToString());
	}
}

void FDlgEditor::OnCommandRedoGraphAction() const
{
	// Clear selection, to avoid holding refs to nodes that go away
	ClearViewportSelection();
	if (!GEditor->RedoTransaction())
	{
		const FText TransactionName = GEditor->Trans->GetUndoContext(true).Title;
		UE_LOG(LogDlgSystemEditor, Warning, TEXT("Redo Transaction with Name = `%s` failed"), *TransactionName.ToString());
	}
}

void FDlgEditor::OnCommandConvertSpeechSequenceNodeToSpeechNodes() const
{
	const TSet<UObject*> SelectedNodes = GetSelectedNodes();
	check(SelectedNodes.Num() == 1);
	UDialogueGraphNode* SpeechSequence_GraphNode = CastChecked<UDialogueGraphNode>(*FDlgHelper::GetFirstSetElement(SelectedNodes));
	check(SpeechSequence_GraphNode->IsSpeechSequenceNode());

	FDlgConvertSpeechSequenceNodeToSpeechNodes_GraphSchemaAction ConvertAction(SpeechSequence_GraphNode);
	ConvertAction.PerformAction(DialogueBeingEdited->GetGraph(), nullptr, SpeechSequence_GraphNode->GetPosition(), false);
}

void FDlgEditor::OnCommandConvertSpeechNodesToSpeechSequence() const
{
	const TSet<UObject*> SelectedNodes = GetSelectedNodes();
	TArray<UDialogueGraphNode*> OutSelectedGraphNodes;
	if (!FDlgEditorUtilities::CanConvertSpeechNodesToSpeechSequence(SelectedNodes, OutSelectedGraphNodes))
	{
		return;
	}

	UDialogueGraphNode* Speech_GraphNode = CastChecked<UDialogueGraphNode>(OutSelectedGraphNodes[0]);
	check(Speech_GraphNode->IsSpeechNode());

	FDlgConvertSpeechNodesToSpeechSequence_GraphSchemaAction ConvertAction(OutSelectedGraphNodes);
	ConvertAction.PerformAction(DialogueBeingEdited->GetGraph(), nullptr, Speech_GraphNode->GetPosition(), false);
}

void FDlgEditor::OnCommandDeleteSelectedNodes() const
{
	const FScopedTransaction Transaction(LOCTEXT("DialogueEditRemoveSelectedNode", "Dialogue Editor: Remove Node"));
	verify(DialogueBeingEdited->Modify());

	const TSet<UObject*> SelectedNodes = GetSelectedNodes();
	UDialogueGraph* DialogueGraph = GetDialogueGraph();
	verify(DialogueGraph->Modify());

	// Keep track of all the node types removed
	int32 NumNodesRemoved = 0;
	int32 NumDialogueNodesRemoved = 0;
	int32 NumBaseDialogueNodesRemoved = 0;
	int32 NumEdgeDialogueNodesRemoved = 0;
	int32 NumStartNodesRemoved = 0;

#if DO_CHECK
	const int32 Initial_DialogueNodesNum = DialogueBeingEdited->GetNodes().Num();
	const int32 Initial_GraphNodesNum = DialogueGraph->Nodes.Num();
	const int32 Initial_GraphDialogueNodesNum = DialogueGraph->GetAllDialogueGraphNodes().Num();
	const int32 Initial_GraphBaseDialogueNodesNum = DialogueGraph->GetAllBaseDialogueGraphNodes().Num();
	const int32 Initial_GraphEdgeDialogueNodesNum = DialogueGraph->GetAllEdgeDialogueGraphNodes().Num();
#endif
	const int32 Initial_StartNodeNum = DialogueGraph->GetRootGraphNodes().Num();

	// Unselect nodes we are about to delete
	ClearViewportSelection();

	// Disable compiling for optimization
	DialogueBeingEdited->DisableCompileDialogue();

	// Helper function to also count the number of removed nodes
	auto RemoveGraphNode = [&NumNodesRemoved](UEdGraphNode* NodeToRemove)
	{
		verify(FDlgEditorUtilities::RemoveNode(NodeToRemove));
		NumNodesRemoved++;
	};

	// Keep track of all the removed edges so that we do not attempt double removal
	TSet<UDialogueGraphNode_Edge*> RemovedEdges;
	auto AddToRemovedEdges = [&RemovedEdges, &NumBaseDialogueNodesRemoved, &NumEdgeDialogueNodesRemoved](UDialogueGraphNode_Edge* NodeEdge)
	{
		RemovedEdges.Add(NodeEdge);
		NumBaseDialogueNodesRemoved++;
		NumEdgeDialogueNodesRemoved++;
	};

	auto RemoveGraphEdgeNode = [&RemovedEdges, &RemoveGraphNode, &AddToRemovedEdges](UDialogueGraphNode_Edge* Edge)
	{
		if (!RemovedEdges.Contains(Edge))
		{
			RemoveGraphNode(Edge);
			AddToRemovedEdges(Edge);
		}
	};

	for (UObject* NodeObject : SelectedNodes)
	{
		UEdGraphNode* SelectedNode = CastChecked<UEdGraphNode>(NodeObject);

		if (!SelectedNode->CanUserDeleteNode())
		{
			continue;
		}

		// only allow root nodes until there is at least one remaining
		if (UDialogueGraphNode_Root* Root = Cast<UDialogueGraphNode_Root>(SelectedNode))
		{
			if (NumStartNodesRemoved == Initial_StartNodeNum - 1)
			{
				continue;
			}

			for (UDialogueGraphNode_Edge* ChildNodeEdge : Root->GetChildEdgeNodes())
			{
				RemoveGraphEdgeNode(ChildNodeEdge);
			}

			// Remove the Node
			// No need to recompile as the the break node links step will do that for us
			verify(FDlgEditorUtilities::RemoveNode(Root));
			NumStartNodesRemoved++;
			NumBaseDialogueNodesRemoved++;
		}
		else if (UDialogueGraphNode_Base* NodeBase = Cast<UDialogueGraphNode_Base>(SelectedNode))
		{
			// Base Node type

			if (UDialogueGraphNode* Node = Cast<UDialogueGraphNode>(NodeBase))
			{
				// Remove the parent/child edges
				for (UDialogueGraphNode_Edge* ParentNodeEdge : Node->GetParentEdgeNodes())
				{
					RemoveGraphEdgeNode(ParentNodeEdge);
				}

				for (UDialogueGraphNode_Edge* ChildNodeEdge : Node->GetChildEdgeNodes())
				{
					RemoveGraphEdgeNode(ChildNodeEdge);
				}

				// Remove the Node
				// No need to recompile as the the break node links step will do that for us
				RemoveGraphNode(Node);
				NumBaseDialogueNodesRemoved++;
				NumDialogueNodesRemoved++;
			}
			else
			{
				// Edge type
				UDialogueGraphNode_Edge* NodeEdge = CastChecked<UDialogueGraphNode_Edge>(NodeBase);
				RemoveGraphEdgeNode(NodeEdge);
			}
		}
		else
		{
			// Most likely it is a comment
			RemoveGraphNode(SelectedNode);
		}
	}

	DialogueBeingEdited->EnableCompileDialogue();
	if (NumBaseDialogueNodesRemoved > 0)
	{
		DialogueBeingEdited->CompileDialogueNodesFromGraphNodes();
		DialogueBeingEdited->PostEditChange();
		DialogueBeingEdited->MarkPackageDirty();
		RefreshViewport();
	}

#if DO_CHECK
	// Check if we removed as we counted
	check(Initial_DialogueNodesNum - NumDialogueNodesRemoved == DialogueBeingEdited->GetNodes().Num());
	check(Initial_GraphNodesNum - NumNodesRemoved - NumStartNodesRemoved == DialogueGraph->Nodes.Num());
	check(Initial_GraphDialogueNodesNum - NumDialogueNodesRemoved - NumStartNodesRemoved == DialogueGraph->GetAllDialogueGraphNodes().Num());
	check(Initial_GraphBaseDialogueNodesNum - NumBaseDialogueNodesRemoved == DialogueGraph->GetAllBaseDialogueGraphNodes().Num());
	check(Initial_GraphEdgeDialogueNodesNum - NumEdgeDialogueNodesRemoved == DialogueGraph->GetAllEdgeDialogueGraphNodes().Num());
	check(NumEdgeDialogueNodesRemoved == RemovedEdges.Num());
	check(Initial_StartNodeNum - NumStartNodesRemoved == DialogueGraph->GetRootGraphNodes().Num());
#endif
}

bool FDlgEditor::CanDeleteNodes() const
{
	const TSet<UObject*>& SelectedNodes = GetSelectedNodes();
	// Return false if only the last root node is selected, as it can't be deleted
	if (SelectedNodes.Num() == 1)
	{
		const UObject* SelectedNode = *FDlgHelper::GetFirstSetElement(SelectedNodes);
		return !SelectedNode->IsA(UDialogueGraphNode_Root::StaticClass()) || GetDialogueGraph()->GetRootGraphNodes().Num() > 0;
	}

	return SelectedNodes.Num() > 0;
}

void FDlgEditor::OnCommandCopySelectedNodes() const
{
	// Export the selected nodes and place the text on the clipboard
	const TSet<UObject*>& SelectedNodes = GetSelectedNodes();
	for (UObject* Object : SelectedNodes)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(Object))
		{
			Node->PrepareForCopying();
		}
	}

	// Copy to clipboard
	FString ExportedText;
	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for (UObject* Object : SelectedNodes)
	{
		if (UDialogueGraphNode_Base* Node = Cast<UDialogueGraphNode_Base>(Object))
		{
			Node->PostCopyNode();
		}
	}
}

bool FDlgEditor::CanCopyNodes() const
{
	// If any of the nodes can be duplicated then we should allow copying
	for (UObject* Object : GetSelectedNodes())
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FDlgEditor::OnCommandPasteNodes()
{
	PasteNodesHere(GraphEditorView->GetPasteLocation());
}

void FDlgEditor::PasteNodesHere(const FVector2D& Location)
{
	// Undo/Redo support
	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
	UDialogueGraph* DialogueGraph = GetDialogueGraph();
	verify(DialogueBeingEdited->Modify());
	verify(DialogueGraph->Modify());

	// Clear the selection set (newly pasted stuff will be selected)
	ClearViewportSelection();

	// Disable compiling for optimization
	DialogueBeingEdited->DisableCompileDialogue();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(DialogueGraph, TextToImport, /*out*/ PastedNodes);

	// Step 1. Calculate average position
	// Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f, 0.0f);
	for (UEdGraphNode* Node : PastedNodes)
	{
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}
	if (PastedNodes.Num() > 0)
	{
		const float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	// maps from old index -> new index
	// Used to remap node edges (FDlgEdge) and checking if some TargetIndex edges are still valid
	TMap<int32, int32> OldToNewIndexMap;
	TArray<UDialogueGraphNode*> GraphNodes;

	// Step 2. Filter Dialogue nodes and position all valid nodes
	for (UEdGraphNode* Node : PastedNodes)
	{
		bool bAddToSelection = true;
		bool bPositionNode = true;


		if (auto* GraphNode = Cast<UDialogueGraphNode>(Node))
		{
			// Add to the dialogue
			const int32 OldNodeIndex = GraphNode->GetDialogueNodeIndex();
			const int32 NewNodeIndex = DialogueBeingEdited->AddNode(GraphNode->GetMutableDialogueNode());
			GraphNode->SetDialogueNodeIndex(NewNodeIndex);

			OldToNewIndexMap.Add(OldNodeIndex, NewNodeIndex);
			GraphNodes.Add(GraphNode);
		}
		else if (UDialogueGraphNode_Edge* Edge = Cast<UDialogueGraphNode_Edge>(Node))
		{
			bPositionNode = false;
			bAddToSelection = Edge->HasParentNode() && Edge->HasChildNode();
		}

		// Select the newly pasted stuff
		if (bAddToSelection)
		{
			GraphEditorView->SetNodeSelection(Node, true);
		}

		if (bPositionNode)
		{
			const float NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			const float NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			if (auto* GraphNodeBase = Cast<UDialogueGraphNode_Base>(Node))
			{
				GraphNodeBase->SetPosition(NodePosX, NodePosY);
			}
			else
			{
				Node->NodePosX = NodePosX;
				Node->NodePosY = NodePosY;
			}
		}

		// Assign new ID
		Node->CreateNewGuid();
	}

	// Step 3. Fix Edges and references of Dialogue/Graph nodes
	// TODO this is similar to what the compiler does, maybe the compiler is too strict? It should not care what the Dialogues have?
	for (UDialogueGraphNode* GraphNode : GraphNodes)
	{
		UDlgNode* DialogueNode = GraphNode->GetMutableDialogueNode();

		const TArray<UDialogueGraphNode_Edge*> GraphNodeChildrenEdges = GraphNode->GetChildEdgeNodes();
		const int32 GraphNodeChildrenEdgesNum = GraphNodeChildrenEdges.Num();
		if (GraphNodeChildrenEdgesNum == 0)
		{
			// There are no links copied, delete the edges from the Dialogue Node.
			DialogueNode->SetNodeChildren({});
		}
		else
		{
			// Fix the dialogue edges
			// Remove node edges to target indicies that were not copied (that do not exist)
			const TArray<FDlgEdge>& OldNodeEdges = DialogueNode->GetNodeChildren();
			TArray<FDlgEdge> NewNodeEdges;
			for (const FDlgEdge& OldEdge : OldNodeEdges)
			{
				// Node was copied over
				if (OldToNewIndexMap.Find(OldEdge.TargetIndex) != nullptr)
				{
					NewNodeEdges.Add(OldEdge);
				}
			}

			DialogueNode->SetNodeChildren(NewNodeEdges);

			// Fix graph edges
			// Remap old indices
			for (int32 ChildIndex = 0; ChildIndex < GraphNodeChildrenEdgesNum; ChildIndex++)
			{
				UDialogueGraphNode_Edge* ChildEdge = GraphNodeChildrenEdges[ChildIndex];
				const int32 OldNodeIndex = ChildEdge->GetDialogueEdge().TargetIndex;
				const int32 NewNodeIndex = OldToNewIndexMap.FindChecked(OldNodeIndex);
				GraphNode->SetEdgeTargetIndexAt(ChildIndex, NewNodeIndex);
			}
		}
	}

	// First fix weak references
	FDlgEditorUtilities::RemapOldIndicesWithNewAndUpdateGUID(GraphNodes, OldToNewIndexMap);

	// Compile
	CheckAll();
	DialogueBeingEdited->EnableCompileDialogue();
	DialogueBeingEdited->CompileDialogueNodesFromGraphNodes();

	// Notify objects of change
	RefreshViewport();
	DialogueBeingEdited->PostEditChange();
	DialogueBeingEdited->MarkPackageDirty();
}

bool FDlgEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(DialogueBeingEdited->GetGraph(), ClipboardContent);
}

void FDlgEditor::OnCommandHideSelectedNodes()
{
	const TSet<UObject*>& SelectedNodes = GetSelectedNodes();
	for (UObject* Object : SelectedNodes)
	{
		if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(Object))
		{
			if (!GraphNode->IsRootNode())
			{
				GraphNode->SetForceHideNode(true);
			}
		}
	}
}

void FDlgEditor::OnCommandUnHideAllNodes()
{
	UDialogueGraph* Graph = GetDialogueGraph();
	for (UDialogueGraphNode* GraphNode : Graph->GetAllDialogueGraphNodes())
	{
		GraphNode->SetForceHideNode(false);
	}
}

void FDlgEditor::OnCommandDialogueReload() const
{
	// Ignore
	if (GetSettings().DialogueTextFormat == EDlgDialogueTextFormat::None)
	{
		return;
	}

	check(DialogueBeingEdited);
	const EAppReturnType::Type Response = FPlatformMisc::MessageBoxExt(EAppMsgType::YesNo,
		TEXT("You are about to overwrite your data in the editor with the data from the text file(.dlg)"),
		TEXT("Overwrite graph/dialogue data from text file?"));
	if (Response == EAppReturnType::No)
	{
		return;
	}

	//const FScopedTransaction Transaction(LOCTEXT("DialogueEditorReloadFromFile", "Dialogue Editor: Reload from file"));

	// Opposite of this steps are in the SaveAsset_Execute
	// Reload data, text file -> dialogue data
	DialogueBeingEdited->ImportFromFile();

	// Update graph, dialogue data -> graph
	DialogueBeingEdited->ClearGraph(); // triggers create default graph nodes
	DialogueBeingEdited->MarkPackageDirty();
}

void FDlgEditor::OnSelectedNodesChanged(const TSet<UObject*>& NewSelection)
{
	TArray<UObject*> ViewSelection;

	PreviousSelectedNodeObjects.Empty();
	if (NewSelection.Num())
	{
		for (UObject* Selected : NewSelection)
		{
			PreviousSelectedNodeObjects.Add(Selected);
			ViewSelection.Add(Selected);
		}
	}
	else
	{
		// Nothing selected, view the properties of this Dialogue.
		ViewSelection.Add(DialogueBeingEdited);
	}

	// View the selected objects
	if (DetailsView.IsValid())
	{
		DetailsView->SetObjects(ViewSelection, /*bForceRefresh=*/ true);
	}

	UpdateNodesHighlightedByProxy(NewSelection);
}

FActionMenuContent FDlgEditor::OnCreateGraphActionMenu(
	UEdGraph* InGraph,
	const FVector2D& InNodePosition,
	const TArray<UEdGraphPin*>& InDraggedPins,
	bool bAutoExpand,
	SGraphEditor::FActionMenuClosed InOnMenuClosed
)
{
	TSharedRef<SDlgDialogueActionMenu> ActionMenu = SNew(SDlgDialogueActionMenu)
		.Graph(InGraph)
		.NewNodePosition(InNodePosition)
		.DraggedFromPins(InDraggedPins)
		.AutoExpandActionMenu(bAutoExpand)
		.OnClosedCallback(InOnMenuClosed)
		.OnCloseReason(this, &Self::OnGraphActionMenuClosed);

	return FActionMenuContent(ActionMenu, ActionMenu->GetFilterTextBox());
}

void FDlgEditor::OnGraphActionMenuClosed(bool bActionExecuted, bool bGraphPinContext)
{
	if (!bActionExecuted)
	{
		// reset previous drag and drop saved edges.
		LastTargetGraphEdgeBeforeDrag = nullptr;
	}
}

void FDlgEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged) const
{
	if (!IsValid(NodeBeingChanged))
	{
		return;
	}

	// Rename the node to the new set text.
	const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Dialogue Editor: Rename Node"));
	verify(NodeBeingChanged->Modify());
	NodeBeingChanged->OnRenameNode(NewText.ToString());
}

void FDlgEditor::UpdateNodesHighlightedByProxy(const TSet<UObject*>& NewSelection)
{
	// gather highlighted node indices based on selection
	TSet<int32> HighlightedNodeIndices;
	for (UObject* NodeObject : NewSelection)
	{
		UDialogueGraphNode* Node = Cast<UDialogueGraphNode>(NodeObject);
		if (Node == nullptr)
		{
			continue;
		}

		if (const UDlgNode_Proxy* Proxy = Cast<UDlgNode_Proxy>(&Node->GetDialogueNode()))
		{
			HighlightedNodeIndices.Add(Proxy->GetTargetNodeIndex());
		}
	}

	// Go through all the nodes in graph, update border highlight if changed
	for (UEdGraphNode* EdGraphNode : DialogueBeingEdited->GetGraph()->Nodes)
	{
		UDialogueGraphNode* Node = Cast<UDialogueGraphNode>(EdGraphNode);
		if (Node == nullptr)
		{
			continue;
		}

		const bool bShouldBeHighlighted = HighlightedNodeIndices.Contains(Node->GetDialogueNodeIndex());
		if (Node->ShouldUseBorderHighlight() != bShouldBeHighlighted)
		{
			Node->SetUseBorderHighlight(bShouldBeHighlighted);
			if (GraphEditorView.IsValid())
			{
				GraphEditorView->RefreshNode(*Node);
			}
		}
	}
}

// End of own functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
