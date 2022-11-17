// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserFrontEndFilterExtension.h"

#include "DlgContentBrowserExtensions.generated.h"

/**
 * Register custom  filters with the content browser
 */
UCLASS()
class DLGSYSTEMEDITOR_API UDlgSearchFilter : public UContentBrowserFrontEndFilterExtension
{
public:
	GENERATED_BODY()

	// UContentBrowserFrontEndFilterExtension interface
	void AddFrontEndFilterExtensions(
		TSharedPtr<FFrontendFilterCategory> DefaultCategory,
		TArray<TSharedRef<FFrontendFilter>>& InOutFilterList
	) const override;
};

/**
 * Integrate DlgSystem actions associated with existing engine types (e.g., DlgSystem) into the content browser.
 * When you right click on a specific asset for example.
 * See Paper2D class for proper example FPaperContentBrowserExtensions
 */
class FDlgContentBrowserExtensions
{
public:
	static void InstallHooks();
	static void RemoveHooks();
};
