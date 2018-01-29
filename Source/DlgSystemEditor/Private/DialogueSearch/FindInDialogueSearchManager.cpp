// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "FindInDialogueSearchManager.h"

#include "Widgets/Docking/SDockTab.h"
#include "AssetRegistryModule.h"
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"

#include "DlgSystemEditorPrivatePCH.h"
#include "DlgDialogue.h"
#include "DlgManager.h"
#include "SFindInDialogues.h"
#include "DialogueEditor/Graph/DialogueGraph.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueStyle.h"

#define LOCTEXT_NAMESPACE "SDialogueBrowser"

FFindInDialogueSearchManager* FFindInDialogueSearchManager::Instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialogueSearchManager
FFindInDialogueSearchManager* FFindInDialogueSearchManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new Self();
	}

	return Instance;
}

FFindInDialogueSearchManager::FFindInDialogueSearchManager()
{
	// Create the Tab Ids
	for (int32 TabIdx = 0; TabIdx < ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = FName(*FString::Printf(TEXT("GlobalDialogueFindResults_%02d"), TabIdx + 1));
		GlobalFindResultsTabIDs[TabIdx] = TabID;
	}
}

FFindInDialogueSearchManager::~FFindInDialogueSearchManager()
{
	UnInitialize();
}

bool FFindInDialogueSearchManager::QueryDlgCondition(const FString& InSearchString, const FDlgCondition& InDlgCondition,
													FFindInDialoguesResultPtr OutParentNode)
{
	if (InSearchString.IsEmpty() || !OutParentNode.IsValid())
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Test ParticipantName
	if (!InDlgCondition.ParticipantName.IsNone() &&
		InDlgCondition.ParticipantName.ToString().Contains(InSearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  FText::FromName(InDlgCondition.ParticipantName),
						  LOCTEXT("DlgConditionParticipantName", "DlgCondition ParticipantName"),
						  TEXT("DlgCondition.ParticipantName"));
	}

	// Test CallBackName
	if (!InDlgCondition.CallbackName.IsNone() &&
		InDlgCondition.CallbackName.ToString().Contains(InSearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  FText::FromName(InDlgCondition.CallbackName),
						  LOCTEXT("DlgConditionCallbackName", "DlgCondition CallbackName"),
						  TEXT("DlgCondition.CallbackName"));
	}

	// Test NameValue
	if (!InDlgCondition.NameValue.IsNone() &&
		InDlgCondition.NameValue.ToString().Contains(InSearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  FText::FromName(InDlgCondition.NameValue),
						  LOCTEXT("DlgConditionNameValue", "DlgCondition NameValue"),
						  TEXT("DlgCondition.NameValue"));
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryDlgEvent(const FString& InSearchString, const FDlgEvent& InDlgEvent,
												FFindInDialoguesResultPtr OutParentNode)
{
	if (InSearchString.IsEmpty() || !OutParentNode.IsValid())
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Test ParticipantName
	if (!InDlgEvent.ParticipantName.IsNone() &&
		InDlgEvent.ParticipantName.ToString().Contains(InSearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  FText::FromName(InDlgEvent.ParticipantName),
						  LOCTEXT("DlgEventParticipantName", "DlgEvent ParticipantName"),
						  TEXT("DlgEvent.ParticipantName"));
	}

	// Test EventName
	if (!InDlgEvent.EventName.IsNone() &&
		InDlgEvent.EventName.ToString().Contains(InSearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  FText::FromName(InDlgEvent.EventName),
						  LOCTEXT("DlgEventCallbackName", "DlgEvent EventName"),
						  TEXT("DlgEvent.EventName"));
	}

	// Test NameValue
	if (!InDlgEvent.NameValue.IsNone() &&
		InDlgEvent.NameValue.ToString().Contains(InSearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  FText::FromName(InDlgEvent.NameValue),
						  LOCTEXT("DlgEventNameValue", "DlgEvent NameValue"),
						  TEXT("DlgEvent.NameValue"));
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryDlgEdge(const FString& InSearchString, const FDlgEdge& InDlgEdge,
												FFindInDialoguesResultPtr OutParentNode)
{
	if (InSearchString.IsEmpty() || !OutParentNode.IsValid())
	{
		return false;
	}
	bool bContainsSearchString = false;


	// Test Text
	if (InDlgEdge.Text.ToString().Contains(InSearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  InDlgEdge.Text,
						  LOCTEXT("DlgEdgText", "DlgEdge Text"),
						  TEXT("DlgEdge.Text"));
	}

	// Test Condition
	for (const FDlgCondition& Condition : InDlgEdge.Conditions)
	{
		bContainsSearchString = bContainsSearchString || QueryDlgCondition(InSearchString, Condition, OutParentNode);
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryGraphNode(const FString& InSearchString, UDialogueGraphNode* InGraphNode,
												FFindInDialoguesResultPtr OutParentNode)
{
	if (InSearchString.IsEmpty() || !OutParentNode.IsValid() || InGraphNode == nullptr)
	{
		return false;
	}

	bool bContainsSearchString = false;
	UDlgNode* Node = InGraphNode->GetMutableDialogueNode();
	const FString NodeType = Node->GetNodeTypeString();

	// Create the GraphNode Node
	const FText DisplayText = FText::Format(LOCTEXT("TreeGraphNodeCategory", "{0} Node at index {1}"),
										 FText::FromString(NodeType), FText::AsNumber(InGraphNode->GetDialogueNodeIndex()));
	TSharedPtr<FFindInDialoguesGraphNode> TreeGraphNode = MakeShareable(new FFindInDialoguesGraphNode(DisplayText, OutParentNode));
	TreeGraphNode->SetCategory(FText::FromString(NodeType));
	TreeGraphNode->SetGraphNode(InGraphNode);

	// Helper method to add children to the TreeGraphNode
	auto AddTextNodeToGraphNode = [this, &bContainsSearchString, &TreeGraphNode](const FText& DisplayName, const FText& Category,
			const FString& CommentString = FString())
	{
		bContainsSearchString = true;
		MakeChildTextNode(TreeGraphNode, DisplayName, Category, CommentString);
	};

	// Text the ParticipantName
	if (Node->GetNodeParticipantName().ToString().Contains(InSearchString))
	{
		AddTextNodeToGraphNode(FText::FromName(Node->GetNodeParticipantName()),
							   LOCTEXT("ParticipantNameKey", "Participant Name"),
							   TEXT("Participant Name"));
	}

	// Test the Node text
	if (Node->GetNodeText().ToString().Contains(InSearchString))
	{
		AddTextNodeToGraphNode(Node->GetNodeText(), LOCTEXT("DescriptionKey", "Description"), TEXT("Description"));
	}

	// Test the EnterConditions
	for (const FDlgCondition& Condition : Node->GetNodeEnterConditions())
	{
		bContainsSearchString = bContainsSearchString || QueryDlgCondition(InSearchString, Condition, TreeGraphNode);
	}

	// Test the EnterEvents
	for (const FDlgEvent& Event : Node->GetNodeEnterEvents())
	{
		bContainsSearchString = bContainsSearchString || QueryDlgEvent(InSearchString, Event, TreeGraphNode);
	}

	// Handle Speech sequences
	if (UDlgNode_SpeechSequence* SpeechSequence = Cast<UDlgNode_SpeechSequence>(Node))
	{
		const TArray<FDlgSpeechSequenceEntry>& SpeechSequenceArray = SpeechSequence->GetNodeSpeechSequence();
		for (int32 Index = 0, Num = SpeechSequenceArray.Num(); Index < Num; Index++)
		{
			const FDlgSpeechSequenceEntry& SequenceEntry = SpeechSequenceArray[Index];

			// Test the Speaker
			if (SequenceEntry.Speaker.ToString().Contains(InSearchString))
			{
				const FText Category = FText::Format(LOCTEXT("SequenceEntrySpeaker", "SequenceEntry.Speaker at index = {0}"),
													 FText::AsNumber(Index));
				AddTextNodeToGraphNode(FText::FromName(SequenceEntry.Speaker), Category, Category.ToString());
			}

			// Test the Text Description
			if (SequenceEntry.Text.ToString().Contains(InSearchString))
			{
				const FText Category = FText::Format(LOCTEXT("SequenceEntryText", "SequenceEntry.Description at index = {0}"),
													 FText::AsNumber(Index));
				AddTextNodeToGraphNode(SequenceEntry.Text, Category, Category.ToString());
			}

			// Test the EdgeText
			if (SequenceEntry.EdgeText.ToString().Contains(InSearchString))
			{
				const FText Category = FText::Format(LOCTEXT("SequenceEntryEdgeText", "SequenceEntry.EdgeText at index = {0}"),
													 FText::AsNumber(Index));
				AddTextNodeToGraphNode(SequenceEntry.EdgeText, Category, Category.ToString());
			}
		}
	}

	if (bContainsSearchString)
	{
		OutParentNode->AddChild(TreeGraphNode);
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryEdgeNode(const FString& InSearchString, UDialogueGraphNode_Edge* InEdgeNode,
												FFindInDialoguesResultPtr OutParentNode)
{
	if (InSearchString.IsEmpty() || !OutParentNode.IsValid() || InEdgeNode == nullptr)
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Build up the Display Text
	int32 FromParent = -1;
	int32 ToChild = -1;
	if (InEdgeNode->HasParentNode())
	{
		FromParent = InEdgeNode->GetParentNode()->GetDialogueNodeIndex();
	}
	if (InEdgeNode->HasChildNode())
	{
		ToChild = InEdgeNode->GetChildNode()->GetDialogueNodeIndex();
	}
	const FText DisplayText = FText::Format(LOCTEXT("EdgeNodeDisplaytext", "Edge between {0} -> {1}"),
											FText::AsNumber(FromParent), FText::AsNumber(ToChild));
	TSharedPtr<FFindInDialoguesEdgeNode> TreeEdgeNode = MakeShareable(new FFindInDialoguesEdgeNode(DisplayText, OutParentNode));
	TreeEdgeNode->SetCategory(DisplayText);
	TreeEdgeNode->SetEdgeNode(InEdgeNode);

	// Search in the DlgEdge
	const FDlgEdge& DialogueEdge = InEdgeNode->GetDialogueEdge();
	bContainsSearchString = bContainsSearchString || QueryDlgEdge(InSearchString, DialogueEdge, TreeEdgeNode);

	if (bContainsSearchString)
	{
		OutParentNode->AddChild(TreeEdgeNode);
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QuerySingleDialogue(const FString& InSearchString,
							const UDlgDialogue* InDialogue, FFindInDialoguesResultPtr OutParentNode)
{
	if (InSearchString.IsEmpty() || !OutParentNode.IsValid() || InDialogue == nullptr)
	{
		return false;
	}

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(InDialogue->GetGraph());
	TArray<UDialogueGraphNode_Base*> BaseNodes = Graph->GetAllBaseDialogueGraphNodes();
	TSharedPtr<FFindInDialoguesDialogueNode> TreeDialogueNode = MakeShareable(
			new FFindInDialoguesDialogueNode(FText::FromString(InDialogue->GetPathName()), OutParentNode));
	TreeDialogueNode->SetDialogue(InDialogue);

	// TODO node comments
	bool bFoundInDialogue = false;
	for (UDialogueGraphNode_Base* BaseNode : BaseNodes)
	{
		bool bFoundInNode = false;
		if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(BaseNode))
		{
			bFoundInNode = QueryGraphNode(InSearchString, GraphNode, TreeDialogueNode);
		}
		else if (UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(BaseNode))
		{
			bFoundInNode = QueryEdgeNode(InSearchString, EdgeNode, TreeDialogueNode);
		}
		else
		{
			checkNoEntry();
		}

		// Found at least one match in one of the nodes.
		bFoundInDialogue = bFoundInDialogue || bFoundInNode;
	}

	if (bFoundInDialogue)
	{
		OutParentNode->AddChild(TreeDialogueNode);
	}

	return bFoundInDialogue;
}

void FFindInDialogueSearchManager::QueryAllDialogues(const FString& InSearchString,
	FFindInDialoguesResultPtr OutParentNode)
{
	// Iterate over all cached dialogues
	for (auto& Elem : SearchMap)
	{
		const FDialogueSearchData& SearchData = Elem.Value;
		if (SearchData.Dialogue.IsValid())
		{
			QuerySingleDialogue(InSearchString, SearchData.Dialogue.Get(), OutParentNode);
		}
	}
}

FText FFindInDialogueSearchManager::GetGlobalFindResultsTabLabel(const int32 TabIdx)
{
	// Count the number of opened global Dialogues
	int32 NumOpenGlobalFindResultsTabs = 0;
	for (int32 i = GlobalFindResultsWidgets.Num() - 1; i >= 0; --i)
	{
		if (GlobalFindResultsWidgets[i].IsValid())
		{
			NumOpenGlobalFindResultsTabs++;
		}
		else
		{
			// Invalid :O remove it
			GlobalFindResultsWidgets.RemoveAt(i);
		}
	}

	if (NumOpenGlobalFindResultsTabs > 1 || TabIdx > 0)
	{
		// Format TabIdx + 1
		return FText::Format(LOCTEXT("GlobalFindResultsTabNameWithIndex", "Find in Dialogues {0}"),
							FText::AsNumber(TabIdx + 1));
	}

	// No Number
	return LOCTEXT("GlobalFindResultsTabName", "Find in Dialogues");
}

void FFindInDialogueSearchManager::CloseGlobalFindResults(const TSharedRef<SFindInDialogues>& FindResults)
{
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		// Remove it from the opened find results
		if (FindResultsPtr.Pin() == FindResults)
		{
			GlobalFindResultsWidgets.Remove(FindResultsPtr);
			break;
		}
	}
}

TSharedRef<SDockTab> FFindInDialogueSearchManager::SpawnGlobalFindResultsTab(
	const FSpawnTabArgs& SpawnTabArgs, const int32 TabIdx)
{
	// Label is Dynamic depending on the number of global tabs
	TAttribute<FText> Label = TAttribute<FText>::Create(
		TAttribute<FText>::FGetter::CreateRaw(this, &Self::GetGlobalFindResultsTabLabel, TabIdx));

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab) // can be docked anywhere
		.Label(Label)
		.ToolTipText(LOCTEXT("GlobalFindResultsTabTooltip", "Search for a string in all Dialogue assets."));

	TSharedRef<SFindInDialogues> FindResults = SNew(SFindInDialogues)
		.bIsSearchWindow(false)
		.ContainingTab(NewTab);

	NewTab->SetContent(FindResults);
	GlobalFindResultsWidgets.Add(FindResults);

	return NewTab;
}

TSharedPtr<SFindInDialogues> FFindInDialogueSearchManager::OpenGlobalFindResultsTab()
{
	TSet<FName> OpenGlobalTabIDs;

	// Check all opened global tabs
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SFindInDialogues> FindResults = FindResultsPtr.Pin();
		if (FindResults.IsValid())
		{
			OpenGlobalTabIDs.Add(FindResults->GetHostTabId());
		}
	}

	// Find the unopened tab, that we can open
	for (int32 Idx = 0; Idx < ARRAY_COUNT(GlobalFindResultsTabIDs); ++Idx)
	{
		const FName GlobalTabId = GlobalFindResultsTabIDs[Idx];
		if (!OpenGlobalTabIDs.Contains(GlobalTabId))
		{
			// GlobalTabId is not opened, open it now
			TSharedRef<SDockTab> NewTab = FGlobalTabmanager::Get()->InvokeTab(GlobalTabId);
			return StaticCastSharedRef<SFindInDialogues>(NewTab->GetContent());
		}
	}

	return TSharedPtr<SFindInDialogues>();
}

TSharedPtr<SFindInDialogues> FFindInDialogueSearchManager::GetGlobalFindResults()
{
	// Find opened find tab
	TSharedPtr<SFindInDialogues> FindResultsToUse;
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SFindInDialogues> FindResults = FindResultsPtr.Pin();
		if (FindResults.IsValid())
		{
			FindResultsToUse = FindResults;
			break;
		}
	}

	if (FindResultsToUse.IsValid())
	{
		// found invoke it
		FGlobalTabmanager::Get()->InvokeTab(FindResultsToUse->GetHostTabId());
	}
	else
	{
		// not found, open a new tab.
		FindResultsToUse = OpenGlobalFindResultsTab();
	}

	return FindResultsToUse;
}

void FFindInDialogueSearchManager::EnableGlobalFindResults(TSharedPtr<FWorkspaceItem> ParentTabCategory)
{
	const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();

	// Register the spawners for all global Find Results tabs
	const FSlateIcon GlobalFindResultsIcon(FDialogueStyle::GetStyleSetName(), FDialogueStyle::PROPERTY_FindDialogueIcon);

	// Add the menu item
	if (!ParentTabCategory.IsValid())
	{
		ParentTabCategory = WorkspaceMenu::GetMenuStructure().GetToolsCategory();
	}

	GlobalFindResultsMenuItem = ParentTabCategory->AddGroup(
		LOCTEXT("WorkspaceMenu_GlobalFindResultsCategory", "Find in Dialogues"),
		LOCTEXT("GlobalFindResultsMenuTooltipText", "Find references to conditions, events, text and variables in all Dialogues."),
		GlobalFindResultsIcon,
		true);

	// Register tab spawners
	for (int32 TabIdx = 0; TabIdx < ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = GlobalFindResultsTabIDs[TabIdx];

		// Tab not registered yet, good.
		if (!GlobalTabManager->CanSpawnTab(TabID))
		{
			const FText DisplayName = FText::Format(LOCTEXT("GlobalFindResultsDisplayName", "Find in Dialogues {0}"),
													FText::AsNumber(TabIdx + 1));
			GlobalTabManager->RegisterNomadTabSpawner(TabID, FOnSpawnTab::CreateRaw(this, &Self::SpawnGlobalFindResultsTab, TabIdx))
				.SetDisplayName(DisplayName)
				.SetIcon(GlobalFindResultsIcon)
				.SetGroup(GlobalFindResultsMenuItem.ToSharedRef());
		}
	}
}

void FFindInDialogueSearchManager::DisableGlobalFindResults()
{
	const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();

	// Close all Global Find Results tabs when turning the feature off, since these may not get closed along with the Blueprint Editor contexts above.
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SFindInDialogues> FindResults = FindResultsPtr.Pin();
		if (FindResults.IsValid())
		{
			FindResults->CloseHostTab();
		}
	}
	GlobalFindResultsWidgets.Empty();

	// Unregister tab spawners
	for (int32 TabIdx = 0; TabIdx < ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = GlobalFindResultsTabIDs[TabIdx];
		if (GlobalTabManager->CanSpawnTab(TabID))
		{
			GlobalTabManager->UnregisterNomadTabSpawner(TabID);
		}
	}

	// Remove the menu item
	if (GlobalFindResultsMenuItem.IsValid())
	{
		WorkspaceMenu::GetMenuStructure().GetToolsCategory()->RemoveItem(GlobalFindResultsMenuItem.ToSharedRef());
		GlobalFindResultsMenuItem.Reset();
	}
}

void FFindInDialogueSearchManager::Initialize(TSharedPtr<FWorkspaceItem> ParentTabCategory)
{
	// Must ensure we do not attempt to load the AssetRegistry Module while saving a package, however, if it is loaded already we can safely obtain it
	if (!GIsSavingPackage || (GIsSavingPackage && FModuleManager::Get().IsModuleLoaded(NAME_MODULE_AssetRegistry)))
	{
		AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(NAME_MODULE_AssetRegistry);
		AssetRegistryModule->Get().OnAssetAdded().AddRaw(this, &Self::HandleAssetAdded);
		AssetRegistryModule->Get().OnAssetRemoved().AddRaw(this, &Self::HandleAssetRemoved);
		AssetRegistryModule->Get().OnAssetRenamed().AddRaw(this, &Self::HandleAssetRenamed);
	}
	else
	{
		// Log a warning to inform the Asset Registry could not be initialized when FiD initialized due to saving package
		// The Asset Registry should be initialized before Find-in-Dialogues, or FiD should be explicitly initialized during a safe time
		// This message will not appear in commandlets because most commandlets do not care. If a search query is made, further warnings will be produced even in commandlets.
		if (!IsRunningCommandlet())
		{
			UE_LOG(LogDlgSystemEditor,
				Warning,
				TEXT("Find-in-Dialogues could not pre-cache all unloaded Dialogues due to the Asset Registry module being unable to initialize because a package is currently being saved. Pre-cache will not be reattempted!"));
		}
	}
	FCoreUObjectDelegates::OnAssetLoaded.AddRaw(this, &Self::HandleAssetLoaded);

	// TODO Pause search if garbage collecting?
	if (!GIsSavingPackage && AssetRegistryModule)
	{
		// Do an immediate load of the cache to catch any Blueprints that were discovered by the asset registry before we initialized.
		BuildCache();
	}

	// Register global find results tabs
	EnableGlobalFindResults(ParentTabCategory);
}

void FFindInDialogueSearchManager::UnInitialize()
{
	if (AssetRegistryModule)
	{
		AssetRegistryModule->Get().OnAssetAdded().RemoveAll(this);
		AssetRegistryModule->Get().OnAssetRemoved().RemoveAll(this);
		AssetRegistryModule->Get().OnAssetRenamed().RemoveAll(this);
	}
	FCoreUObjectDelegates::OnAssetLoaded.RemoveAll(this);

	// Shut down the global find results tab feature.
	DisableGlobalFindResults();
}

void FFindInDialogueSearchManager::BuildCache()
{
	// Difference between this and the UDlgManger::GetAllDialoguesFromMemory is that this loads all Dialogues
	// even those that are not loaded into memory.
	// TODO this seems slow :(
	// AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	// FARFilter ClassFilter;
	// ClassFilter.bRecursiveClasses = true;
	// ClassFilter.ClassNames.Add(UDlgDialogue::StaticClass()->GetFName());
	// TArray<FAssetData> DialogueAssets;
	// AssetRegistryModule->Get().GetAssets(ClassFilter, DialogueAssets);
	// for (FAssetData& Asset : DialogueAssets)
	// {
	// 	HandleAssetAdded(Asset);
	// }

	// We already loaded all Dialogues into memory in the StartupModule.
	for (UDlgDialogue* Dialogue : UDlgManager::GetAllDialoguesFromMemory())
	{
		FAssetData AssetData(Dialogue);
		HandleAssetAdded(AssetData);
	}
}

void FFindInDialogueSearchManager::HandleAssetAdded(const FAssetData& InAssetData)
{
	// Confirm that the Dialogue has not been added already, this can occur during duplication of Dialogues.
	FDialogueSearchData* SearchDataPtr = SearchMap.Find(InAssetData.ObjectPath);
	if (SearchDataPtr != nullptr)
	{
		// Already exists
		return;
	}

	// Ignore other assets
	if (InAssetData.GetClass() && !InAssetData.GetClass()->IsChildOf<UDlgDialogue>())
	{
		return;
	}

	// Load the Dialogue
	UDlgDialogue* Dialogue = Cast<UDlgDialogue>(InAssetData.GetAsset());
	if (Dialogue == nullptr)
	{
		return;
	}

	// Add to the loaded cached map
	FDialogueSearchData SearchData;
	SearchData.Dialogue = Dialogue;
	SearchMap.Add(InAssetData.ObjectPath, MoveTemp(SearchData));
}

void FFindInDialogueSearchManager::HandleAssetRemoved(const FAssetData& InAssetData)
{
	// TODO
}

void FFindInDialogueSearchManager::HandleAssetRenamed(const FAssetData& InAssetData, const FString& InOldName)
{
	// TODO
}

void FFindInDialogueSearchManager::HandleAssetLoaded(UObject* InAsset)
{

}

#undef LOCTEXT_NAMESPACE
