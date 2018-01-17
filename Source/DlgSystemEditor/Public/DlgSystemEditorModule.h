// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "ModuleInterface.h"
#include "SharedPointer.h"
#include "AssetTypeCategories.h"
#include "IAssetTypeActions.h"
#include "Commands/UICommandList.h"
#include "Docking/WorkspaceItem.h"

class FSpawnTabArgs;
class UK2Node;
class UDlgDialogue;

class FDlgSystemEditorModule : public IModuleInterface
{
	typedef FDlgSystemEditorModule Self;
public:
	FDlgSystemEditorModule();

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

	// Own functions
	/**
	 * Tries to get the dialogue name... it expects the owner of the node to implement IDlgDialogueParticipant interface
	 * @return		the participant name on success or NAME_None on failure.
	 */
	static FName GetParticipantNameFromNode(UK2Node* Node);

	/** Self explanatory what the method does */
	static bool DoesObjectImplementDialogueParticipantInterface(UObject* Object);

	/**
	 * Save all the dialogues.
	 * @return True on success or false on failure.
	 */
	static bool SaveAllDialogues();

private:
	/** Handle creating the Dialogue Overview Browser. */
	static TSharedRef<SDockTab> HandleSpawnDialogueBrowser(const FSpawnTabArgs& SpawnTabArgs);

	/** Handle clicking on save all dialogues. */
	static void HandleOnSaveAllDialogues();

	/** Extend the Menus of the editor */
	void ExtendMenu();

private:
	/** The submenu type of the dialog system */
	EAssetTypeCategories::Type DlgSystemAssetCategoryBit;

	/** All registered asset type actions. Cached here so that we can unregister them during shutdown. */
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** All registered custom class layouts for the details panel. Cached here so that we can unregister them during shutdown. */
	TArray<FName> RegisteredCustomClassLayouts;

	/** All registered custom property layouts for the details panel. Cached here so that we can unregister them during shutdown. */
	TArray<FName> RegisteredCustomPropertyTypeLayout;

	/** The factory of how the nodes look. */
	TSharedPtr<struct FGraphPanelNodeFactory> DialogueGraphNodeFactory;

	/** The factory of how the pins look. */
	TSharedPtr<struct FGraphPanelPinFactory> DialogueGraphPinFactory;

	/**
	 * Global Editor commands bound from this plugin.
	 */
	TSharedPtr<FUICommandList> GlobalEditorCommands;

	/** The Tools Dialogue category. */
	TSharedPtr<FWorkspaceItem> ToolsDialogueCategory;
};
