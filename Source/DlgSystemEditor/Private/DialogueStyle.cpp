// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueStyle.h"

#include "SlateStyle.h"
#include "IPluginManager.h"
#include "SlateStyleRegistry.h"

#include "DlgSystemEditorPrivatePCH.h"
#include "DlgSystemModule.h"

// Const icon sizes
static const FVector2D Icon16x16(16.0f, 16.0f);
static const FVector2D Icon20x20(20.0f, 20.0f);
static const FVector2D Icon24x24(24.0f, 24.0f);
static const FVector2D Icon40x40(40.0f, 40.0f);
static const FVector2D Icon64x64(64.0f, 64.0f);
static const FVector2D Icon96x96(96.0f, 96.0f);


// Initialize static variables
const FName FDialogueStyle::PROPERTY_DialogueClassIcon(TEXT("ClassIcon.DlgDialogue"));
const FName FDialogueStyle::PROPERTY_DialogueClassThumbnail(TEXT("ClassThumbnail.DlgDialogue"));
const FName FDialogueStyle::PROPERTY_GraphNodeCircleBox(TEXT("DlgSystemEditor.Graph.Node.Circle"));
const FName FDialogueStyle::PROPERTY_ConditionIcon(TEXT("DlgSystemEditor.Condition.Icon"));
const FName FDialogueStyle::PROPERTY_EventIcon(TEXT("DlgSystemEditor.Event.Icon"));
const FName FDialogueStyle::PROPERTY_QuestionMarkIcon(TEXT("DlgSystemEditor.QuestionMark.Icon"));

// Tied with FDialogueEditorCommands::DialogueReloadData
const FName FDialogueStyle::PROPERTY_ReloadAssetIcon(TEXT("DlgSystemEditor.DialogueReloadData"));
const FName FDialogueStyle::PROPERTY_OpenAssetIcon(TEXT("DlgSystemEditor.OpenAsset.Icon"));
const FName FDialogueStyle::PROPERTY_FindAssetIcon(TEXT("DlgSystemEditor.FindAsset.Icon"));

// Tied with FDialogueEditorCommands::SaveAllDialogues
const FName FDialogueStyle::PROPERTY_SaveAllDialoguesIcon(TEXT("DlgSystemEditor.SaveAllDialogues"));
const FName FDialogueStyle::PROPERTY_FindDialogueIcon(TEXT("DlgSystemEditor.FindDialogue.Icon"));
const FName FDialogueStyle::PROPERTY_BrowseDialogueIcon(TEXT("DlgSystemEditor.BrowseDialogue.Icon"));

// Tied with FDialogueEditorCommands::FindInDialogue
const FName FDialogueStyle::PROPERTY_FindInDialogueEditorIcon(TEXT("DlgSystemEditor.FindInDialogue"));

// Tied with FDialogueEditorCommands::FindInAllDialogues
const FName FDialogueStyle::PROPERTY_FindInAllDialogueEditorIcon(TEXT("DlgSystemEditor.FindInAllDialogues"));

TSharedPtr<FSlateStyleSet> FDialogueStyle::StyleSet = nullptr;

void FDialogueStyle::Initialize()
{
	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	// register the current style
	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	TSharedPtr<IPlugin> CurrentPlugin = IPluginManager::Get().FindPlugin(DIALOGUE_SYSTEM_PLUGIN_NAME);
	if (CurrentPlugin.IsValid())
	{
		// Replaces the Engine Content Root (Engine/Editor/Slate) with the plugin content root
		StyleSet->SetContentRoot(CurrentPlugin->GetContentDir());
	}
	else
	{
		UE_LOG(LogDlgSystemEditor, Fatal, TEXT("Could not find the Dialogue System Plugin :("));
		return;
	}

#define IMAGE_PLUGIN_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ... ) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__ )

	StyleSet->Set(PROPERTY_DialogueClassIcon, new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_16x", Icon16x16));
	StyleSet->Set(PROPERTY_DialogueClassThumbnail, new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_64x", Icon64x64));
	StyleSet->Set(PROPERTY_ReloadAssetIcon, new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_Reload_40x", Icon40x40));
	StyleSet->Set(GetSmallProperty(PROPERTY_ReloadAssetIcon), new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_Reload_16x", Icon16x16));
	StyleSet->Set("DlgSystemEditor.ToggleShowPrimarySecondaryEdges", new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_ShowPrimarySecondaryEdges_40x", Icon40x40));
	StyleSet->Set("DlgSystemEditor.ToggleShowPrimarySecondaryEdges.Small", new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_ShowPrimarySecondaryEdges_40x", Icon16x16));

	StyleSet->Set(PROPERTY_FindInDialogueEditorIcon, new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_Find_40x", Icon40x40));
	StyleSet->Set(GetSmallProperty(PROPERTY_FindInDialogueEditorIcon), new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_Find_40x", Icon20x20));

	StyleSet->Set(PROPERTY_FindInAllDialogueEditorIcon, new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_FindAll_40x", Icon40x40));
	StyleSet->Set(GetSmallProperty(PROPERTY_FindInAllDialogueEditorIcon), new IMAGE_PLUGIN_BRUSH("Icons/Dialogue_FindAll_40x", Icon20x20));

	// Level Editor Save All Dialogues
	StyleSet->Set(PROPERTY_SaveAllDialoguesIcon, new IMAGE_PLUGIN_BRUSH("Icons/SaveDialogue_16x", Icon16x16));
	StyleSet->Set(PROPERTY_FindDialogueIcon, new IMAGE_PLUGIN_BRUSH("Icons/FindDialogue_16x", Icon16x16));
	StyleSet->Set(PROPERTY_BrowseDialogueIcon, new IMAGE_PLUGIN_BRUSH("Icons/BrowseDialogue_16x", Icon16x16));

	// Set common used properties
	StyleSet->Set(PROPERTY_ConditionIcon, new IMAGE_PLUGIN_BRUSH("Icons/Condition_96x", Icon96x96));
	StyleSet->Set(PROPERTY_EventIcon, new IMAGE_PLUGIN_BRUSH("Icons/Event_96x", Icon96x96));
	StyleSet->Set(PROPERTY_QuestionMarkIcon, new IMAGE_PLUGIN_BRUSH("Icons/QuestionMark_16x", Icon16x16));
	StyleSet->Set(PROPERTY_OpenAssetIcon, new IMAGE_PLUGIN_BRUSH("Icons/OpenAsset_16px", Icon16x16));
	StyleSet->Set(PROPERTY_FindAssetIcon, new IMAGE_PLUGIN_BRUSH("Icons/FindAsset_16x", Icon16x16));
	StyleSet->Set(PROPERTY_GraphNodeCircleBox, new BOX_BRUSH("Icons/Circle_20x", Icon20x20, FMargin(8.0f / 20.0f)));

#undef IMAGE_PLUGIN_BRUSH
#undef BOX_BRUSH

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FDialogueStyle::Shutdown()
{
	// unregister the style
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

