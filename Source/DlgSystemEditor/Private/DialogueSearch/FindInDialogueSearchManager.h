// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"

#include "FindInDialoguesResult.h"

// The maximum amount of global Dialogue Search windows opened.
static constexpr int32 MAX_GLOBAL_DIALOGUE_SEARCH_RESULTS = 4;

class SFindInDialogues;
class FAssetRegistryModule;

class FWorkspaceItem;
class UDialogueGraphNode;
class UDialogueGraphNode_Edge;
class UEdGraphNode_Comment;
struct FAssetData;
struct FDlgCondition;
struct FDlgEvent;
struct FDlgEdge;
struct FDlgTextArgument;

struct FDialogueSearchData
{
	/** The Dialogue this search data points to, if available */
	TWeakObjectPtr<UDlgDialogue> Dialogue;
};

/** Singleton manager for handling all Dialogue searches */
class FFindInDialogueSearchManager
{
private:
	typedef FFindInDialogueSearchManager Self;

public:
	static Self* Get();

	FFindInDialogueSearchManager();
	~FFindInDialogueSearchManager();

	/**
	 * Searches for InSearchString in the InDlgTextArgument. Adds the result as a child in OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QueryDlgTextArgument(const FDialogueSearchFilter& SearchFilter, const FDlgTextArgument& InDlgTextArgument,
		TSharedPtr<FFindInDialoguesResult> OutParentNode, const int32 ArgumentIndex = INDEX_NONE);

	/**
	 * Searches for InSearchString in the InDlgCondition. Adds the result as a child in OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QueryDlgCondition(const FDialogueSearchFilter& SearchFilter, const FDlgCondition& InDlgCondition,
		TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/**
	 * Searches for InSearchString in the InDlgEvent. Adds the result as a child in OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QueryDlgEvent(const FDialogueSearchFilter& SearchFilter, const FDlgEvent& InDlgEvent,
		TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/**
	 * Searches for InSearchString in the InDlgEdge. Adds the result as a child in OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QueryDlgEdge(const FDialogueSearchFilter& SearchFilter, const FDlgEdge& InDlgEdge,
		TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/**
	 * Searches for InSearchString in the InGraphNode. Adds the result as a child in OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QueryGraphNode(const FDialogueSearchFilter& SearchFilter, const UDialogueGraphNode* InGraphNode,
		TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/**
	 * Searches for InSearchString in the InEdgeNode. Adds the result as a child in OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QueryEdgeNode(const FDialogueSearchFilter& SearchFilter, const UDialogueGraphNode_Edge* InEdgeNode,
		TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/**
	 * Searches for InSearchString in the Comment Node. Adds the result as a child in OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QueryCommentNode(const FDialogueSearchFilter& SearchFilter, const UEdGraphNode_Comment* InCommentNode,
		TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/**
	 * Searches for InSearchString in the InDialogue. Adds the result as a child of OutParentNode.
	 * @return True if found anything matching the InSearchString
	 */
	bool QuerySingleDialogue(const FDialogueSearchFilter& SearchFilter,
		const UDlgDialogue* InDialogue, TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/**
	 * Searches for InSearchString in all Dialogues. Adds the result as children of OutParentNode.
	 */
	void QueryAllDialogues(const FDialogueSearchFilter& SearchFilter, TSharedPtr<FFindInDialoguesResult> OutParentNode);

	/** Determines the global find results tab label */
	FText GetGlobalFindResultsTabLabel(const int32 TabIdx);

	/** Close One of the global find results. */
	void CloseGlobalFindResults(const TSharedRef<SFindInDialogues>& FindResults);

	/** Find or create the global find results widget */
	TSharedPtr<SFindInDialogues> GetGlobalFindResults();

	/** Enables the global find results tab feature in the Windows Menu. */
	void EnableGlobalFindResults(TSharedPtr<FWorkspaceItem> ParentTabCategory = nullptr);

	/** Disables the global find results tab feature in the Windows Menu. */
	void DisableGlobalFindResults();

	/** Initializes the manager. Should only be called once in the FDlgSystemEditorModule::StartupModule()  */
	void Initialize(TSharedPtr<FWorkspaceItem> ParentTabCategory = nullptr);

	/** Uninitializes the manager. Should only be called once in the FDlgSystemEditorModule::ShutdownModule()  */
	void UnInitialize();

private:
	/** Helper method to make a Text Node and add it as a child to ParentNode */
	TSharedPtr<FFindInDialoguesResult> MakeChildTextNode(TSharedPtr<FFindInDialoguesResult> ParentNode, const FText& DisplayName, const FText& Category, const FString& CommentString = FString())
	{
		TSharedPtr<FFindInDialoguesResult> TextNode = MakeShareable(new FFindInDialoguesResult(DisplayName, ParentNode));
		TextNode->SetCategory(Category);
		if (!CommentString.IsEmpty())
		{
			TextNode->SetCommentString(CommentString);
		}
		ParentNode->AddChild(TextNode);
		return TextNode;
	}

	/** Handler for a request to spawn a new global find results tab */
	TSharedRef<SDockTab> SpawnGlobalFindResultsTab(const FSpawnTabArgs& SpawnTabArgs, const int32 TabIdx);

	/** Creates and opens a new global find results tab. The next one in the available list. */
	TSharedPtr<SFindInDialogues> OpenGlobalFindResultsTab();

	/** Builds the cache from all available Dialogues assets that the asset registry has discovered at the time of this function. Occurs on startup. */
	void BuildCache();

	/** Callback hook from the Asset Registry when an asset is added */
	void HandleAssetAdded(const FAssetData& InAssetData);

	/** Callback hook from the Asset Registry, marks the asset for deletion from the cache */
	void HandleAssetRemoved(const FAssetData& InAssetData);

	/** Callback hook from the Asset Registry, marks the asset for deletion from the cache */
	void HandleAssetRenamed(const FAssetData& InAssetData, const FString& InOldName);

	/** Callback hook from the Asset Registry when an asset is loaded */
	void HandleAssetLoaded(UObject* InAsset);

private:
	static Self* Instance;

	/** Maps the Dialogue path => SearchData. */
	TMap<FName, FDialogueSearchData> SearchMap;

	/** Because we are unable to query for the module on another thread, cache it for use later */
	FAssetRegistryModule* AssetRegistryModule;

	/** The tab identifier/instance name for global find results */
	FName GlobalFindResultsTabIDs[MAX_GLOBAL_DIALOGUE_SEARCH_RESULTS];

	/** Array of open global find results widgets */
	TArray<TWeakPtr<SFindInDialogues>> GlobalFindResultsWidgets;

	/** Global Find Results workspace menu item */
	TSharedPtr<FWorkspaceItem> GlobalFindResultsMenuItem;
};
