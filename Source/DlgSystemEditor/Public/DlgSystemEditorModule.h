// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "IDlgSystemEditorModule.h"
#include "Templates/SharedPointer.h"
#include "AssetTypeCategories.h"
#include "IAssetTypeActions.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/WorkspaceItem.h"


class FSpawnTabArgs;
class UK2Node;
class UDlgDialogue;
struct FGraphPanelNodeFactory;
struct FGraphPanelPinFactory;

/**
 * Implementation of the DlgSystemEditor Module
 */
class DLGSYSTEMEDITOR_API FDlgSystemEditorModule : public IDlgSystemEditorModule
{
	typedef FDlgSystemEditorModule Self;
public:
	FDlgSystemEditorModule();

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

	// Own functions
	/**
	 * Save all the dialogues.
	 * @return True on success or false on failure.
	 */
	static bool SaveAllDialogues();

private:
	/** Handle clicking on save all dialogues. */
	static void HandleOnSaveAllDialogues();

	/** Handle on post engine init event */
	void HandleOnPostEngineInit();

	/** Extend the Menus of the editor */
	void HandleOnBeginPIE(bool bIsSimulating);

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
	TSharedPtr<FGraphPanelNodeFactory> DialogueGraphNodeFactory;

	/** The factory of how the pins look. */
	TSharedPtr<FGraphPanelPinFactory> DialogueGraphPinFactory;

	/**
	 * Global Editor commands bound from this plugin.
	 */
	TSharedPtr<FUICommandList> GlobalEditorCommands;

	/** The Tools Dialogue category. */
	TSharedPtr<FWorkspaceItem> ToolsDialogueCategory;

	// Handlers
	FDelegateHandle OnPostEngineInitHandle;
	FDelegateHandle OnBeginPIEHandle;
};
