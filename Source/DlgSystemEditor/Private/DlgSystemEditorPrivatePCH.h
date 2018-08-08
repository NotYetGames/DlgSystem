// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgSystemEditorModule.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgSystemEditor, Verbose, All)

const FName DIALOGUE_SYSTEM_MENU_CATEGORY_KEY(TEXT("Dialogue System"));
const FText DIALOGUE_SYSTEM_MENU_CATEGORY_KEY_TEXT(NSLOCTEXT("DlgSystemEditor", "DlgSystemAssetCategory", "Dialogue System"));

// Other Modules constants
static const FName NAME_MODULE_AssetTools(TEXT("AssetTools"));
static const FName NAME_MODULE_AssetRegistry(TEXT("AssetRegistry"));
static const FName NAME_MODULE_LevelEditor(TEXT("LevelEditor"));
static const FName NAME_MODULE_PropertyEditor(TEXT("PropertyEditor"));
