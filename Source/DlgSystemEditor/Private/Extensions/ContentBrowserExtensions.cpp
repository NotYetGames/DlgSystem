// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "ContentBrowserExtensions.h"

#include "ContentBrowserModule.h"
#include "Engine/Blueprint.h"

#include "DlgManager.h"
#include "DlgSystemEditorPrivatePCH.h"

#define LOCTEXT_NAMESPACE "DialogueSystemContentBrowserExtensions"


FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
FDelegateHandle ContentBrowserExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////
// FContentBrowserSelectedAssetExtensionBase
struct FContentBrowserSelectedAssetExtensionBase
{
public:
	TArray<FAssetData> SelectedAssets;

public:
	virtual void Execute() {}
	virtual ~FContentBrowserSelectedAssetExtensionBase() {}
};

//////////////////////////////////////////////////////////////////////////
// FFrontendFilter_DialogueParticipants
/** A filter that search for blueprints that have implemented the Dialogue Participant */
class FFrontendFilter_DialogueParticipants : public FFrontendFilter
{
public:
	FFrontendFilter_DialogueParticipants(const TSharedPtr<FFrontendFilterCategory>& InCategory)
		: FFrontendFilter(InCategory)
	{
	}

	// FFrontendFilter implementation
	/** Returns the system name for this filter */
	FString GetName() const override
	{
		return TEXT("Dialogue Participant Filter");
	}

	/** Returns the human readable name for this filter */
	FText GetDisplayName() const override
	{
		return LOCTEXT("FilterDialogueParticipants_Name", "Dialogue Participants");
	}

	/** Returns the tooltip for this filter, shown in the filters menu */
	FText GetToolTipText() const override
	{
		return LOCTEXT("FilterDialogueParticipants_ToolTip", "Search for any Blueprints that implement the Dialogue Participant Interface.");
	}

	/** Returns the color this filter button will be when displayed as a button */
	FLinearColor GetColor() const override  { return FLinearColor::Yellow; }

	/** Returns the name of the icon to use in menu entries */
	FName GetIconName() const override { return NAME_None; }

	/** Invoke to set the ARFilter that is currently used to filter assets in the asset view */
	void SetCurrentFilter(const FARFilter& InBaseFilter) override { }
	// End of FFrontendFilter implementation

	// IFilter implementation
	/** Returns whether the specified Item passes the Filter's restrictions */
	bool PassesFilter(FAssetFilterType InItem) const override
	{
		if (!InItem.IsAssetLoaded())
		{
			return false;
		}

		if (const UObject* Object = InItem.GetAsset())
		{
			return UDlgManager::DoesObjectImplementDialogueParticipantInterface(Object);
		}

		return false;
	}
	// End of IFilter implementation
};

//////////////////////////////////////////////////////////////////////////
// UDialogueSearchFilter
void UDialogueSearchFilter::AddFrontEndFilterExtensions(TSharedPtr<FFrontendFilterCategory> DefaultCategory,
	TArray<TSharedRef<FFrontendFilter>>& InOutFilterList) const
{
	TSharedPtr<FFrontendFilterCategory> DialogueCategory =
		MakeShared<FFrontendFilterCategory>(LOCTEXT("DlgSystemCategoryName", "Dialogue System Filters"),
											LOCTEXT("DlgSystemCategoryTooltip", "Filter Dialogue System assets"));

	InOutFilterList.Add(MakeShared<FFrontendFilter_DialogueParticipants>(DialogueCategory));
}


//////////////////////////////////////////////////////////////////////////
// FDlgContentBrowserExtensions
void FDlgContentBrowserExtensions::InstallHooks()
{

}

void FDlgContentBrowserExtensions::RemoveHooks()
{

}

#undef LOCTEXT_NAMESPACE
