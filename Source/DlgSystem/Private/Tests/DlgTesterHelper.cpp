// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgTesterHelper.h"


std::function<FString(const int32&)> FDlgTestHelper::IntToString = [](const int32& Value) -> FString
{
	return FString::FromInt(Value);
};


std::function<FString(const FName&)> FDlgTestHelper::NameToString = [](const FName& Value) -> FString
{
	return Value.ToString();
};

std::function<FString(const FString&)> FDlgTestHelper::StringToString = [](const FString& Value) -> FString
{
	return Value;
};

std::function<FString(const float&)> FDlgTestHelper::FloatToString = [](const float& Value) -> FString
{
	return FString::SanitizeFloat(Value);
};

std::function<FString(const FVector&)> FDlgTestHelper::VectorToString = [](const FVector& Value) -> FString
{
	return Value.ToString();
};

std::function<FString(const FColor&)> FDlgTestHelper::ColorToString = [](const FColor& Value) -> FString
{
	return Value.ToString();
};
