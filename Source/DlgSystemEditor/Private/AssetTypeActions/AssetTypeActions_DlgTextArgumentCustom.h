// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "AssetTypeActions_Base.h"

#include "DlgTextArgumentCustom.h"

class IToolkitHost;

/**
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class FAssetTypeActions_DlgTextArgumentCustom : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_DlgTextArgumentCustom(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	//
	// IAssetTypeActions interface
	//

	// Overriden by the Factory
	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "DlgTextArgumentCustomAssetTypeActions", "Dialogue Custom Text Argument Blueprint"); }
	FColor GetTypeColor() const override { return FColor( 63, 126, 255 ); }
	UClass* GetSupportedClass() const override { return UDlgTextArgumentCustom::StaticClass(); }
	bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
	uint32 GetCategories() override { return AssetCategory; }

private:
	EAssetTypeCategories::Type AssetCategory;
};
