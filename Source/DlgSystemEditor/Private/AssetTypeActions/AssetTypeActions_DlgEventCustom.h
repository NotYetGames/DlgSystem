// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "AssetTypeActions_Base.h"

#include "DlgEventCustom.h"

class IToolkitHost;

/**
 * See FDlgSystemEditorModule::StartupModule for usage.
 */
class FAssetTypeActions_DlgEventCustom : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_DlgEventCustom(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	//
	// IAssetTypeActions interface
	//

	// Overriden by the Factory
	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "DlgEventCustomAssetTypeActions", "Dialogue Custom Event Blueprint"); }
	FColor GetTypeColor() const override { return FColor( 63, 126, 255 ); }
	UClass* GetSupportedClass() const override { return UDlgEventCustom::StaticClass(); }
	bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
	uint32 GetCategories() override { return AssetCategory; }

private:
	EAssetTypeCategories::Type AssetCategory;
};
