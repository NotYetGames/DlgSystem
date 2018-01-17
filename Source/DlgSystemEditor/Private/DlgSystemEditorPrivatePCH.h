// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "DlgSystemEditorModule.h"
#include "LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgSystemEditor, Verbose, All)

const FName DIALOGUE_SYSTEM_MENU_CATEGORY_KEY(TEXT("Dialogue System"));
const FText DIALOGUE_SYSTEM_MENU_CATEGORY_KEY_TEXT(NSLOCTEXT("DlgSystemEditor", "DlgSystemAssetCategory", "Dialogue System"));

// Other Modules constants
static const FName NAME_MODULE_AssetTools(TEXT("AssetTools"));
static const FName NAME_MODULE_AssetRegistry(TEXT("AssetRegistry"));
static const FName NAME_MODULE_LevelEditor(TEXT("LevelEditor"));
static const FName NAME_MODULE_PropertyEditor(TEXT("PropertyEditor"));
