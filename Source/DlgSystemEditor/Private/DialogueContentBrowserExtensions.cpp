// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueContentBrowserExtensions.h"

#include "ContentBrowserModule.h"
#include "DlgConstants.h"
#include "DlgEventCustom.h"
#include "Engine/Blueprint.h"

#include "DlgManager.h"
#include "DlgSystemEditorModule.h"

#define LOCTEXT_NAMESPACE "DialogueSystemContentBrowserExtensions"


// //////////////////////////////////////////////////////////////////////////
// // A filter that searches for Dialogues
// // We use this because it allows us more flexibility FAssetTypeActions_DlgDialogue
// // NOTE: Because this is a frontend filter we can't combine it with the other backend filters (blueprint, texture, particles, etc)
// // Unreal does not allow you to have custom backend filters, which is kinda retarded
// class FFrontendFilter_Dialogue : public FFrontendFilter
// {
// public:
//     FFrontendFilter_Dialogue(const TSharedPtr<FFrontendFilterCategory>& InCategory)
//         : FFrontendFilter(InCategory)
//     {
//     }
//
// 	//
// 	// FFrontendFilter implementation
// 	//
//
// 	FString GetName() const override
//     {
//     	return TEXT("Dialogue");
//     }
// 	FText GetDisplayName() const override
//     {
//     	return LOCTEXT("FilterDialogue_Name", "Dialogue");
//     }
// 	FText GetToolTipText() const override
//     {
//     	return LOCTEXT("FilterDialogue_ToolTip", "Filter By Dialogue");
//     }
//
// 	FLinearColor GetColor() const override  { return FLinearColor::Yellow; }
// 	FName GetIconName() const override { return NAME_None; }
//
// 	void SetCurrentFilter(const FARFilter& InBaseFilter) override
//     {
//
//     }
//
// 	//
// 	// IFilter implementation
// 	//
//
// 	/** Returns whether the specified Item passes the Filter's restrictions */
// 	bool PassesFilter(FAssetFilterType InItem) const override
//     {
//     	return InItem.GetClass() == UDlgDialogue::StaticClass();
//     }
// };

//////////////////////////////////////////////////////////////////////////
// A filter that search for blueprints that have implemented the Dialogue Participant
class FFrontendFilter_DialogueParticipants : public FFrontendFilter
{
public:
	FFrontendFilter_DialogueParticipants(const TSharedPtr<FFrontendFilterCategory>& InCategory)
		: FFrontendFilter(InCategory)
	{
	}

	//
	// FFrontendFilter implementation
	//

	FString GetName() const override
	{
		return TEXT("Dialogue Participant Filter");
	}
	FText GetDisplayName() const override
	{
		return LOCTEXT("FilterDialogueParticipants_Name", "Dialogue Participant");
	}
	FText GetToolTipText() const override
	{
		return LOCTEXT("FilterDialogueParticipants_ToolTip", "Search for any Blueprints that implement the Dialogue Participant Interface");
	}

	FLinearColor GetColor() const override  { return FLinearColor(0.91f, 0.91f, 0.f); }
	FName GetIconName() const override { return NAME_None; }

	void SetCurrentFilter(const FARFilter& InBaseFilter) override
	{

	}

	//
	// IFilter implementation
	//

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
};

// //////////////////////////////////////////////////////////////////////////
// // A filter that search for Custom Events
// class FFrontendFilter_DialogueCustomEvent : public FFrontendFilter
// {
// public:
//     FFrontendFilter_DialogueCustomEvent(const TSharedPtr<FFrontendFilterCategory>& InCategory)
//         : FFrontendFilter(InCategory)
//     {
//     }
//
// 	//
// 	// FFrontendFilter implementation
// 	//
//
// 	FString GetName() const override
//     {
//     	return TEXT("Dialogue Custom Event");
//     }
// 	FText GetDisplayName() const override
//     {
//     	return LOCTEXT("FilterDialogueCustomEvent_Name", "Dialogue Custom Event");
//     }
// 	FText GetToolTipText() const override
//     {
//     	return LOCTEXT("FilterDialogueCustomEvent__ToolTip", "Search for any Blueprints that is a Dialogue Custom Event");
//     }
//
// 	// Orange
// 	FLinearColor GetColor() const override  { return FLinearColor(1.f, 0.46f, 0.f); }
// 	FName GetIconName() const override { return NAME_None; }
//
// 	void SetCurrentFilter(const FARFilter& InBaseFilter) override
//     {
//
//     }
//
// 	//
// 	// IFilter implementation
// 	//
//
// 	/** Returns whether the specified Item passes the Filter's restrictions */
// 	bool PassesFilter(FAssetFilterType InItem) const override
//     {
//     	if (!InItem.IsAssetLoaded())
//     	{
//     		return false;
//     	}
//
//     	if (const UObject* Object = InItem.GetAsset())
//     	{
//     		return UDlgManager::IsObjectACustomEvent(Object);
//     	}
//
//     	return false;
//     }
// };
//
// //////////////////////////////////////////////////////////////////////////
// // A filter that search for Custom Events
// class FFrontendFilter_DialogueCustomCondition : public FFrontendFilter
// {
// public:
//     FFrontendFilter_DialogueCustomCondition(const TSharedPtr<FFrontendFilterCategory>& InCategory)
//         : FFrontendFilter(InCategory)
//     {
//     }
//
// 	//
// 	// FFrontendFilter implementation
// 	//
//
// 	FString GetName() const override
//     {
//     	return TEXT("Dialogue Custom Condtition");
//     }
// 	FText GetDisplayName() const override
//     {
//     	return LOCTEXT("FilterDialogueCustomCondition_Name", "Dialogue Custom Condition");
//     }
// 	FText GetToolTipText() const override
//     {
//     	return LOCTEXT("FilterDialogueCustomCondition_ToolTip", "Search for any Blueprints that is a Dialogue Custom Condition");
//     }
//
// 	// Orange
// 	FLinearColor GetColor() const override  { return FLinearColor(1.f, 0.46f, 0.f); }
// 	FName GetIconName() const override { return NAME_None; }
//
// 	void SetCurrentFilter(const FARFilter& InBaseFilter) override
//     {
//
//     }
//
// 	//
// 	// IFilter implementation
// 	//
//
// 	/** Returns whether the specified Item passes the Filter's restrictions */
// 	bool PassesFilter(FAssetFilterType InItem) const override
//     {
//     	if (!InItem.IsAssetLoaded())
//     	{
//     		return false;
//     	}
//
//     	if (const UObject* Object = InItem.GetAsset())
//     	{
//     		return UDlgManager::IsObjectACustomCondition(Object);
//     	}
//
//     	return false;
//     }
// };
//
// //////////////////////////////////////////////////////////////////////////
// // A filter that search for Custom Text Argument
// class FFrontendFilter_DialogueCustomTextArgument : public FFrontendFilter
// {
// public:
//     FFrontendFilter_DialogueCustomTextArgument(const TSharedPtr<FFrontendFilterCategory>& InCategory)
//         : FFrontendFilter(InCategory)
//     {
//     }
//
// 	//
// 	// FFrontendFilter implementation
// 	//
//
// 	FString GetName() const override
//     {
//     	return TEXT("Dialogue Custom Text Argument");
//     }
// 	FText GetDisplayName() const override
//     {
//     	return LOCTEXT("FilterDialogueCustomTextArgument_Name", "Dialogue Custom Text Argument");
//     }
// 	FText GetToolTipText() const override
//     {
//     	return LOCTEXT("FilterDialogueCustomTextArgument_ToolTip", "Search for any Blueprints that is a Dialogue Custom Text Argument");
//     }
//
// 	// Orange
// 	FLinearColor GetColor() const override  { return FLinearColor(1.f, 0.46f, 0.f); }
// 	FName GetIconName() const override { return NAME_None; }
//
// 	void SetCurrentFilter(const FARFilter& InBaseFilter) override
//     {
//
//     }
//
// 	//
// 	// IFilter implementation
// 	//
//
// 	/** Returns whether the specified Item passes the Filter's restrictions */
// 	bool PassesFilter(FAssetFilterType InItem) const override
//     {
//     	if (!InItem.IsAssetLoaded())
//     	{
//     		return false;
//     	}
//
//     	if (const UObject* Object = InItem.GetAsset())
//     	{
//     		return UDlgManager::IsObjectACustomTextArgument(Object);
//     	}
//
//     	return false;
//     }
// };

//////////////////////////////////////////////////////////////////////////
// UDialogueSearchFilter
void UDialogueSearchFilter::AddFrontEndFilterExtensions(
	TSharedPtr<FFrontendFilterCategory> DefaultCategory,
	TArray<TSharedRef<FFrontendFilter>>& InOutFilterList
) const
{
	// TSharedPtr<FFrontendFilterCategory> DialogueCategory = MakeShared<FFrontendFilterCategory>(
	// 	LOCTEXT("DlgSystemCategoryName", "Dialogue System Filters"),
	// 	LOCTEXT("DlgSystemCategoryTooltip", "Filter Dialogue System assets")
	// );
	TSharedPtr<FFrontendFilterCategory> DialogueCategory = MakeShared<FFrontendFilterCategory>(
	     FText::FromName(OTHER_DIALOGUE_SYSTEM_MENU_CATEGORY_KEY),
	     OTHER_DIALOGUE_SYSTEM_MENU_CATEGORY_KEY_TEXT
	);

	// NOTE: we can't combine these with multiple filters
	// InOutFilterList.Add(MakeShared<FFrontendFilter_Dialogue>(DialogueCategory));
	InOutFilterList.Add(MakeShared<FFrontendFilter_DialogueParticipants>(DialogueCategory));
	//InOutFilterList.Add(MakeShared<FFrontendFilter_DialogueCustomEvent>(DialogueCategory));
	//InOutFilterList.Add(MakeShared<FFrontendFilter_DialogueCustomCondition>(DialogueCategory));
	//InOutFilterList.Add(MakeShared<FFrontendFilter_DialogueCustomTextArgument>(DialogueCategory));
}


//////////////////////////////////////////////////////////////////////////
// FDialogueContentBrowserExtensions
void FDialogueContentBrowserExtensions::InstallHooks()
{

}

void FDialogueContentBrowserExtensions::RemoveHooks()
{

}

#undef LOCTEXT_NAMESPACE
