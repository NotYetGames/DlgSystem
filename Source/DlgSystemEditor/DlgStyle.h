// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Styling/SlateStyle.h"

// how everything looks, fancy stuff
class DLGSYSTEMEDITOR_API FDlgStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static TSharedPtr<ISlateStyle> Get() { return StyleSet; }

	/** Gets the style name. */
	static FName GetStyleSetName() { return TEXT("DlgSystemStyle"); }

	/** Gets the small property name variant */
	static FName GetSmallProperty(FName PropertyName)
	{
		return FName(*(PropertyName.ToString() + TEXT(".Small")));
	}

	/** Get the RelativePath to the DlgSystem Content Dir */
	static FString GetPluginContentPath(const FString& RelativePath)
	{
		return PluginContentRoot / RelativePath;
	}

	/** Get the RelativePath to the Engine Content Dir */
	static FString GetEngineContentPath(const FString& RelativePath)
	{
		return EngineContentRoot / RelativePath;
	}

public:
	static const FName PROPERTY_DlgDialogueClassIcon;
	static const FName PROPERTY_DlgDialogueClassThumbnail;

	static const FName PROPERTY_DlgEventCustomClassIcon;
	static const FName PROPERTY_DlgEventCustomClassThumbnail;

	static const FName PROPERTY_DlgConditionCustomClassIcon;
	static const FName PROPERTY_DlgConditionCustomClassThumbnail;

	static const FName PROPERTY_DlgTextArgumentCustomClassIcon;
	static const FName PROPERTY_DlgTextArgumentCustomClassThumbnail;

	static const FName PROPERTY_DlgNodeDataClassIcon;
	static const FName PROPERTY_DlgNodeDataClassThumbnail;

	static const FName PROPERTY_GraphNodeCircleBox;
	static const FName PROPERTY_ConditionIcon;
	static const FName PROPERTY_EventIcon;
	static const FName PROPERTY_VoiceIcon;
	static const FName PROPERTY_GenericIcon;
	static const FName PROPERTY_QuestionMarkIcon;

	static const FName PROPERTY_ShowPrimarySecondaryEdgesIcon;
	static const FName PROPERTY_ReloadAssetIcon;
	static const FName PROPERTY_OpenAssetIcon;
	static const FName PROPERTY_FindAssetIcon;

	static const FName PROPERTY_SaveAllDialoguesIcon;
	static const FName PROPERTY_DeleteAllDialoguesTextFilesIcon;
	static const FName PROPERTY_DeleteCurrentDialogueTextFilesIcon;
	static const FName PROPERTY_DialogueSearch_TabIcon;
	static const FName PROPERTY_DialogueBrowser_TabIcon;
	static const FName PROPERTY_DialogueDataDisplay_TabIcon;

	static const FName PROPERTY_NotYetLogoIcon;
	static const FName PROPERTY_OpenDiscordIcon;
	static const FName PROPERTY_OpenNotYetPluginsIcon;
	static const FName PROPERTY_OpenForumIcon;
	static const FName PROPERTY_OpenMarketplaceIcon;
	static const FName PROPERTY_OpenWikiIcon;

	static const FName PROPERTY_FindInDialogueEditorIcon;
	static const FName PROPERTY_FindInAllDialogueEditorIcon;

	static const FName PROPERTY_CommentBubbleOn;

private:
	/** Singleton instance. */
	static TSharedPtr<FSlateStyleSet> StyleSet;

	/** Engine content root. */
	static FString EngineContentRoot;

	/** DlgSystem Content Root */
	static FString PluginContentRoot;
};
