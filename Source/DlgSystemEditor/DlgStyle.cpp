// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgStyle.h"

#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#include "DlgSystemEditorModule.h"
#include "DlgSystem/DlgSystemModule.h"

// Const icon sizes
static const FVector2D Icon16x16(16.0f, 16.0f);
static const FVector2D Icon20x20(20.0f, 20.0f);
static const FVector2D Icon24x24(24.0f, 24.0f);
static const FVector2D Icon40x40(40.0f, 40.0f);
static const FVector2D Icon64x64(64.0f, 64.0f);
static const FVector2D Icon96x96(96.0f, 96.0f);


// What is displayed in the Content Browser
const FName FDlgStyle::PROPERTY_DlgDialogueClassIcon(TEXT("ClassIcon.DlgDialogue"));
const FName FDlgStyle::PROPERTY_DlgDialogueClassThumbnail(TEXT("ClassThumbnail.DlgDialogue"));
const FName FDlgStyle::PROPERTY_DlgEventCustomClassIcon(TEXT("ClassIcon.DlgEventCustom"));
const FName FDlgStyle::PROPERTY_DlgEventCustomClassThumbnail(TEXT("ClassThumbnail.DlgEventCustom"));
const FName FDlgStyle::PROPERTY_DlgConditionCustomClassIcon(TEXT("ClassIcon.DlgConditionCustom"));
const FName FDlgStyle::PROPERTY_DlgConditionCustomClassThumbnail(TEXT("ClassThumbnail.DlgConditionCustom"));
const FName FDlgStyle::PROPERTY_DlgTextArgumentCustomClassIcon(TEXT("ClassIcon.DlgTextArgumentCustom"));
const FName FDlgStyle::PROPERTY_DlgTextArgumentCustomClassThumbnail(TEXT("ClassThumbnail.DlgTextArgumentCustom"));
const FName FDlgStyle::PROPERTY_DlgNodeDataClassIcon(TEXT("ClassIcon.DlgNodeData"));
const FName FDlgStyle::PROPERTY_DlgNodeDataClassThumbnail(TEXT("ClassThumbnail.DlgNodeData"));


// Inside the Dialogue Editor Window
const FName FDlgStyle::PROPERTY_GraphNodeCircleBox(TEXT("DlgSystemEditor.Graph.Node.Circle"));
const FName FDlgStyle::PROPERTY_ConditionIcon(TEXT("DlgSystemEditor.Condition"));
const FName FDlgStyle::PROPERTY_EventIcon(TEXT("DlgSystemEditor.Event"));
const FName FDlgStyle::PROPERTY_VoiceIcon(TEXT("DlgSystemEditor.Voice"));
const FName FDlgStyle::PROPERTY_GenericIcon(TEXT("DlgSystemEditor.Generic"));
const FName FDlgStyle::PROPERTY_QuestionMarkIcon(TEXT("DlgSystemEditor.QuestionMark"));

// Tied with FDlgCommands::DialogueReloadData
const FName FDlgStyle::PROPERTY_ReloadAssetIcon(TEXT("DlgSystemEditor.DialogueReloadData"));

// Tied with FDlgCommands::ToggleShowPrimarySecondaryEdges
const FName FDlgStyle::PROPERTY_ShowPrimarySecondaryEdgesIcon(TEXT("DlgSystemEditor.ToggleShowPrimarySecondaryEdges"));

// For the Dialogue Browser
const FName FDlgStyle::PROPERTY_OpenAssetIcon(TEXT("DlgSystemEditor.OpenAsset"));
const FName FDlgStyle::PROPERTY_FindAssetIcon(TEXT("DlgSystemEditor.FindAsset"));
const FName FDlgStyle::PROPERTY_DialogueBrowser_TabIcon(TEXT("DlgSystemEditor.DialogueBrowser.TabIcon"));

// Tied with FDlgCommands::SaveAllDialogues
const FName FDlgStyle::PROPERTY_SaveAllDialoguesIcon(TEXT("DlgSystemEditor.SaveAllDialogues"));

// Tied with FDlgCommands::DeleteAllDialoguesTextFiles
const FName FDlgStyle::PROPERTY_DeleteAllDialoguesTextFilesIcon(TEXT("DlgSystemEditor.DeleteAllDialoguesTextFiles"));

// Tied with FDlgCommands::DeleteCurrentDialogueTextFiles
const FName FDlgStyle::PROPERTY_DeleteCurrentDialogueTextFilesIcon(TEXT("DlgSystemEditor.DeleteCurrentDialogueTextFiles"));

// For the Dialogue Search Browser
const FName FDlgStyle::PROPERTY_DialogueSearch_TabIcon(TEXT("DlgSystemEditor.DialogueSearch.TabIcon"));

// For the Dialogue Data Display Window
const FName FDlgStyle::PROPERTY_DialogueDataDisplay_TabIcon(TEXT("DlgSystemEditor.DialogueDataDisplay.TabIcon"));

// Tied with FDlgCommands::FindInDialogue
const FName FDlgStyle::PROPERTY_FindInDialogueEditorIcon(TEXT("DlgSystemEditor.FindInDialogue"));

// Tied with FDlgCommands::FindInAllDialogues
const FName FDlgStyle::PROPERTY_FindInAllDialogueEditorIcon(TEXT("DlgSystemEditor.FindInAllDialogues"));

// Tied with FDlgCommands::OpenDiscord
const FName FDlgStyle::PROPERTY_OpenDiscordIcon(TEXT("DlgSystemEditor.OpenDiscord"));

// Tied with FDlgCommands::OpenForum
const FName FDlgStyle::PROPERTY_OpenForumIcon(TEXT("DlgSystemEditor.OpenForum"));

// Tied with FDlgCommands::OpenNotYetPlugins
const FName FDlgStyle::PROPERTY_OpenNotYetPluginsIcon(TEXT("DlgSystemEditor.OpenNotYetPlugins"));

// Tied with FDlgCommands::OpenMarketplace
const FName FDlgStyle::PROPERTY_OpenMarketplaceIcon(TEXT("DlgSystemEditor.OpenMarketplace"));

// Tied with FDlgCommands::OpenWiki
const FName FDlgStyle::PROPERTY_OpenWikiIcon(TEXT("DlgSystemEditor.OpenWiki"));

// For FDlgSearchResult_CommentNode
const FName FDlgStyle::PROPERTY_CommentBubbleOn(TEXT("DlgSystemEditor.CommentBubbleOn"));

const FName FDlgStyle::PROPERTY_NotYetLogoIcon(TEXT("DlgSystemEditor.NotYetLogo.Icon"));

// The private ones
TSharedPtr<FSlateStyleSet> FDlgStyle::StyleSet = nullptr;
FString FDlgStyle::EngineContentRoot = FString();
FString FDlgStyle::PluginContentRoot = FString();

void FDlgStyle::Initialize()
{
	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShared<FSlateStyleSet>(GetStyleSetName());
	EngineContentRoot = FPaths::EngineContentDir() / TEXT("Editor/Slate");
	TSharedPtr<IPlugin> CurrentPlugin = IPluginManager::Get().FindPlugin(DIALOGUE_SYSTEM_PLUGIN_NAME.ToString());
	if (CurrentPlugin.IsValid())
	{
		// Replaces the Engine Content Root (Engine/Editor/Slate) with the plugin content root
		StyleSet->SetContentRoot(CurrentPlugin->GetContentDir());
		PluginContentRoot = CurrentPlugin->GetContentDir();
	}
	else
	{
		UE_LOG(LogDlgSystemEditor, Fatal, TEXT("Could not find the Dialogue System Plugin :("));
		return;
	}

	// Content Browser icons for asset types
	StyleSet->Set(
		PROPERTY_DlgDialogueClassIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgDialogue_16x.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_DlgDialogueClassThumbnail,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgDialogue_64x.png"), Icon64x64)
	);
	StyleSet->Set(
		PROPERTY_DlgEventCustomClassIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgEventCustom_16x.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_DlgEventCustomClassThumbnail,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgEventCustom_64x.png"), Icon64x64)
	);
	StyleSet->Set(
		PROPERTY_DlgConditionCustomClassIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgConditionCustom_16x.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_DlgConditionCustomClassThumbnail,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgConditionCustom_64x.png"), Icon64x64)
	);
	StyleSet->Set(
		PROPERTY_DlgTextArgumentCustomClassIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgTextArgumentCustom_16x.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_DlgTextArgumentCustomClassThumbnail,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgTextArgumentCustom_64x.png"), Icon64x64)
	);
	StyleSet->Set(
		PROPERTY_DlgNodeDataClassIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgNodeData_16x.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_DlgNodeDataClassThumbnail,
		new FSlateImageBrush(GetPluginContentPath("Icons/DlgNodeData_64x.png"), Icon64x64)
	);

	// Dialogue Search
	StyleSet->Set(
		PROPERTY_FindInDialogueEditorIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_Blueprint_Find_40px.png"), Icon40x40)
	);
	StyleSet->Set(
		GetSmallProperty(PROPERTY_FindInDialogueEditorIcon),
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_Blueprint_Find_40px.png"), Icon20x20)
	);

	StyleSet->Set(
		PROPERTY_FindInAllDialogueEditorIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_FindInAnyBlueprint_40px.png"), Icon40x40)
	);
	StyleSet->Set(
		GetSmallProperty(PROPERTY_FindInAllDialogueEditorIcon),
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_FindInAnyBlueprint_40px.png"), Icon20x20)
	);

	StyleSet->Set(
		PROPERTY_DialogueSearch_TabIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_Genericfinder_16x.png"), Icon16x16)
	);

	// Level Editor File
	StyleSet->Set(
		PROPERTY_SaveAllDialoguesIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_file_saveall_40x.png"), Icon40x40)
	);
	StyleSet->Set(
		PROPERTY_DeleteAllDialoguesTextFilesIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/Edit/icon_Edit_Delete_40x.png"), Icon40x40)
	);
	StyleSet->Set(
		PROPERTY_DeleteCurrentDialogueTextFilesIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/Edit/icon_Edit_Delete_40x.png"), Icon40x40)
	);

	// Level Editor Help
	StyleSet->Set(
		PROPERTY_OpenDiscordIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Discord_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_OpenNotYetPluginsIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/NotYet_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_OpenForumIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Forum_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_OpenWikiIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Wiki_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_OpenMarketplaceIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Marketplace_96x.png"), Icon96x96)
	);

	// Dialogue Browser
	StyleSet->Set(
		PROPERTY_DialogueBrowser_TabIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_tab_ContentBrowser_16x.png"), Icon16x16)
	);

	// Dialogue Data Display
	StyleSet->Set(
		PROPERTY_DialogueDataDisplay_TabIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/DebugTools_40x.png"), Icon16x16)
	);

	// Dialogue Editor Window
	StyleSet->Set(
		PROPERTY_ReloadAssetIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_Cascade_RestartInLevel_40x.png"), Icon40x40)
	);
	StyleSet->Set(
		GetSmallProperty(PROPERTY_ReloadAssetIcon),
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_Refresh_16x.png"), Icon16x16)
	);

	StyleSet->Set(
		PROPERTY_ShowPrimarySecondaryEdgesIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Dialogue_ShowPrimarySecondaryEdges_40x.png"), Icon40x40)
	);
	StyleSet->Set(
		GetSmallProperty(PROPERTY_ShowPrimarySecondaryEdgesIcon),
		new FSlateImageBrush(GetPluginContentPath("Icons/Dialogue_ShowPrimarySecondaryEdges_40x.png"), Icon16x16)
	);

	// Custom Icon sizes, original image is 128x64, we divide by 1.4
	StyleSet->Set(
		PROPERTY_NotYetLogoIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/NotYet_128x_64x.png"), FVector2D{91.42f, 45.8f})
	);
	StyleSet->Set(
		GetSmallProperty(PROPERTY_NotYetLogoIcon),
		new FSlateImageBrush(GetPluginContentPath("Icons/NotYet_96x.png"), Icon16x16)
	);

	StyleSet->Set(
		PROPERTY_ConditionIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Condition_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_EventIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Event_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_VoiceIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Speaker_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_GenericIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/Generic_96x.png"), Icon96x96)
	);
	StyleSet->Set(
		PROPERTY_QuestionMarkIcon,
		new FSlateImageBrush(GetPluginContentPath("Icons/QuestionMark_16x.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_OpenAssetIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_asset_open_16px.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_FindAssetIcon,
		new FSlateImageBrush(GetEngineContentPath("Icons/icon_Genericfinder_16x.png"), Icon16x16)
	);
	StyleSet->Set(
		PROPERTY_GraphNodeCircleBox,
		new FSlateBoxBrush(
			GetEngineContentPath("BehaviorTree/IndexCircle.png"),
			Icon20x20,
			FMargin(8.0f / 20.0f)
		)
	);
	StyleSet->Set(
		PROPERTY_CommentBubbleOn,
		new FSlateImageBrush(
			GetEngineContentPath("Icons/icon_Blueprint_CommentBubbleOn_16x.png"),
			Icon16x16,
			FLinearColor(1.f, 1.f, 1.f, 1.f)
		)
	);

	// Register the current style
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FDlgStyle::Shutdown()
{
	// unregister the style
	if (!StyleSet.IsValid())
	{
		return;
	}

	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
	ensure(StyleSet.IsUnique());
	StyleSet.Reset();
}
