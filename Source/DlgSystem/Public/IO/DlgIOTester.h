// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreTypes.h"
#include "LogMacros.h"
#include "TextProperty.h"

#include "DlgIOTesterTypes.h"

/** 
 *		WIP test class for parsers
 *		TODO: TMap tests, UObject* tests
 */
class DLGSYSTEM_API DlgIOTester
{
public:

	template <typename ConfigWriterType, typename ConfigParserType>
	static bool TestParser();

private:

	template <typename ConfigWriterType, typename ConfigParserType, typename StructType>
	static bool TestStruct(const FString& StructDescription);
};


template <typename ConfigWriterType, typename ConfigParserType>
bool DlgIOTester::TestParser()
{
	bool bAllSucceeded = true;

	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestStructPrimitives>("Struct of Primitives") && bAllSucceeded;
	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestArrayPrimitive>("Array of Primitives") && bAllSucceeded;
	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestStructPrimitives>("Array of Structs") && bAllSucceeded;
	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestArrayEnum>("Array of Enums") && bAllSucceeded;

	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestSetPrimitive>("Set of Primitives") && bAllSucceeded;
	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestSetStruct>("Set of Structs") && bAllSucceeded;
	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestSetEnum>("Set of Enums") && bAllSucceeded;

	return bAllSucceeded;
}


template <typename ConfigWriterType, typename ConfigParserType, typename StructType>
bool DlgIOTester::TestStruct(const FString& StructDescription)
{
	StructType ExportedStruct;
	StructType ImportedStruct;

	ConfigWriterType Writer(StructType::StaticStruct(), &ExportedStruct);

	ConfigParserType Parser;
	Parser.InitializeParserFromString(Writer.GetAsString());
	Parser.ReadAllProperty(StructType::StaticStruct(), &ImportedStruct);

	if (ExportedStruct == ImportedStruct)
	{
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Test Failed: %s"), *StructDescription);
	return false;
}
