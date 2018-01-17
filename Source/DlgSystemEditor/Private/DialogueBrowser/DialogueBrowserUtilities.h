// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

enum class EDialogueBrowserSortOption : uint8
{
	Name = 0,
	DialogueReferences
};

struct FDialogueBrowserSortOption
{

public:
	FDialogueBrowserSortOption(const EDialogueBrowserSortOption InOption, const FName& InName)
		: Option(InOption), Name(InName) {}

	FName GetFName() const { return Name; }
	FText GetFText() const { return FText::FromName(Name); }
	FString GetFString() const { return Name.ToString(); }

	bool IsByName() const { return Option == EDialogueBrowserSortOption::Name; }

public:
	EDialogueBrowserSortOption Option;

	/** The name of the option. */
	FName Name;

	// TODO add ascending descending
};

