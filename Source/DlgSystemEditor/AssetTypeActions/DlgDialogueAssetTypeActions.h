// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "AssetTypeActions_Base.h"

#include "DlgSystem/DlgDialogue.h"

class IToolkitHost;

/**
 * Defines the look and actions the editor takes when clicking/viewing a Dialogue asset.
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class FDlgDialogueAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FDlgDialogueAssetTypeActions(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	//
	// IAssetTypeActions interface
	//

	/** Returns the name of this type */
	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "DlgDialogueAssetTypeActions", "Dialogue"); }

	/** Returns the color associated with this type */
	FColor GetTypeColor() const override { return FColor::Yellow; }

	/** Checks to see if the specified object is handled by this type. */
	UClass* GetSupportedClass() const override { return UDlgDialogue::StaticClass(); }

	// if the menu has actions from another asset, like create dialogue from this text file or idk
	/** Returns true if this class can supply actions for InObjects. */
	bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }

	/** Generates a menubuilder for the specified objects. */
	void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

	/** Opens the asset editor for the specified objects. If EditWithinLevelEditor is valid, the world-centric editor will be used. */
	void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	/** Returns the categories that this asset type. The return value is one or more flags from EAssetTypeCategories.  */
	uint32 GetCategories() override { return AssetCategory; }

	// Hide from filtered, we use our custom FFrontendFilter_Dialogue because it allows us more flexibility
	bool CanFilter() override { return true; }

private:
	/** Indicates the category used for Dialogues */
	EAssetTypeCategories::Type AssetCategory;
};
