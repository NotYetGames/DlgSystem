// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "AssetTypeActions_Base.h"
#include "DlgConditionCustom.h"
#include "DlgEventCustom.h"

#include "DlgNodeData.h"
#include "DlgTextArgumentCustom.h"
#include "DialogueEditorUtilities.h"
#include "DlgHelper.h"

class IToolkitHost;

/**
 * See FDlgSystemEditorModule::StartupModule for usage.
 * NOTE: all of these are Blueprints but we derrive it here so that it appears nicer in the content browser
 */
class FAssetTypeActions_DlgBlueprintDerived : public FAssetTypeActions_Base
{
public:
	//
	// IAssetTypeActions interface
	//
	FAssetTypeActions_DlgBlueprintDerived(EAssetTypeCategories::Type InAssetCategory) : AssetCategory(InAssetCategory) {}

	// Same Color as the Blueprints
	FColor GetTypeColor() const override { return FColor(63, 126, 255); }
	bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
	uint32 GetCategories() override { return AssetCategory; }
	bool CanFilter() override { return true; }
	void BuildBackendFilter(FARFilter& InFilter) override
	{
		FilterAddNativeParentClassPath(InFilter, GetSupportedClass());

		// Add to filter all native children classes of our supported class
		TArray<UClass*> NativeChildClasses;
		TArray<UClass*> BlueprintChildClasses;
		FDlgHelper::GetAllChildClassesOf(GetSupportedClass(), NativeChildClasses, BlueprintChildClasses);
		for (const UClass* ChildNativeClass : NativeChildClasses)
		{
			FilterAddNativeParentClassPath(InFilter, ChildNativeClass);
		}

		InFilter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
		InFilter.bRecursiveClasses = true;
	}

	static void FilterAddNativeParentClassPath(FARFilter& InFilter, const UClass* Class)
	{
		if (Class == nullptr)
		{
			return;
		}

		const FString Value = FString::Printf(
			TEXT("%s'%s'"),
			*UClass::StaticClass()->GetName(),
			*Class->GetPathName()
		);
		InFilter.TagsAndValues.Add(FBlueprintTags::NativeParentClassPath, Value);
	}

protected:
	EAssetTypeCategories::Type AssetCategory;
};


class FAssetTypeActions_DlgNodeData : public FAssetTypeActions_DlgBlueprintDerived
{
public:
	//
	// IAssetTypeActions interface
	//
	FAssetTypeActions_DlgNodeData(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_DlgBlueprintDerived(InAssetCategory) {}

	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "DlgNodeDataAssetTypeActions", "Dialogue Node Data Blueprint"); }
	UClass* GetSupportedClass() const override { return UDlgNodeData::StaticClass(); }
};


class FAssetTypeActions_DlgEventCustom : public FAssetTypeActions_DlgBlueprintDerived
{
public:
	//
	// IAssetTypeActions interface
	//
	FAssetTypeActions_DlgEventCustom(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_DlgBlueprintDerived(InAssetCategory) {}

	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "DlgEventCustomAssetTypeActions", "Dialogue Custom Event Blueprint"); }
	UClass* GetSupportedClass() const override { return UDlgEventCustom::StaticClass(); }
};


class FAssetTypeActions_DlgConditionCustom : public FAssetTypeActions_DlgBlueprintDerived
{
public:
	//
	// IAssetTypeActions interface
	//
	FAssetTypeActions_DlgConditionCustom(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_DlgBlueprintDerived(InAssetCategory) {}

	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "DlgConditionCustomAssetTypeActions", "Dialogue Custom Condition Blueprint"); }
	UClass* GetSupportedClass() const override { return UDlgConditionCustom::StaticClass(); }
};


class FAssetTypeActions_DlgTextArgumentCustom : public FAssetTypeActions_DlgBlueprintDerived
{
public:
	//
	// IAssetTypeActions interface
	//
	FAssetTypeActions_DlgTextArgumentCustom(EAssetTypeCategories::Type InAssetCategory) : FAssetTypeActions_DlgBlueprintDerived(InAssetCategory) {}

	FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "DlgTextArgumentCustomAssetTypeActions", "Dialogue Custom Text Argument Blueprint"); }
	UClass* GetSupportedClass() const override { return UDlgTextArgumentCustom::StaticClass(); }
};
