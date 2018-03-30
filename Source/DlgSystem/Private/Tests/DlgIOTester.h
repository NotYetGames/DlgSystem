// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreTypes.h"
#include "LogMacros.h"
#include "TextProperty.h"

#include "DlgIOTesterTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgIOTester, All, All);


/**
 *	WIP test class for parsers
 *	TODO: UObject* tests
 */
class DLGSYSTEM_API FDlgIOTester
{
public:
	// Tests Parser and Writer
	template <typename ConfigWriterType, typename ConfigParserType>
	static bool TestParser(const FDlgIOTesterOptions& Options, const FString NameWriterType = FString(),
						   const FString NameParserType = FString());

	// Test all parsers/writers
	static bool TestAllParsers();

	template <typename ConfigWriterType, typename ConfigParserType, typename StructType>
	static bool TestStruct(const FString& StructDescription,
						   const FDlgIOTesterOptions& Options,
						   const FString NameWriterType = FString(),
						   const FString NameParserType = FString());
};


template <typename ConfigWriterType, typename ConfigParserType>
bool FDlgIOTester::TestParser(const FDlgIOTesterOptions& Options, const FString NameWriterType, const FString NameParserType)
{
	bool bAllSucceeded = true;

	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestStructPrimitives>("Struct of Primitives", Options, NameWriterType, NameParserType)
		&& bAllSucceeded;

	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestArrayPrimitive>("Array of Primitives", Options, NameWriterType, NameParserType)
		&& bAllSucceeded;
	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestArrayStruct>("Array of Structs", Options, NameWriterType, NameParserType)
		&& bAllSucceeded;

	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestSetPrimitive>("Set of Primitives", Options, NameWriterType, NameParserType)
		&& bAllSucceeded;
	if (Options.bSupportsNonPrimitiveInSet)
	{
		bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestSetStruct>("Set of Structs", Options, NameWriterType, NameParserType)
			&& bAllSucceeded;
	}

	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestMapPrimitive>("Map with Primitives", Options, NameWriterType, NameParserType)
		&& bAllSucceeded;
	bAllSucceeded = TestStruct<ConfigWriterType, ConfigParserType, FDlgTestMapStruct>("Map with Structs", Options, NameWriterType, NameParserType)
		&& bAllSucceeded;

	return bAllSucceeded;
}


template <typename ConfigWriterType, typename ConfigParserType, typename StructType>
bool FDlgIOTester::TestStruct(const FString& StructDescription, const FDlgIOTesterOptions& Options, const FString NameWriterType, const FString NameParserType)
{
	StructType ExportedStruct(Options);
	StructType ImportedStruct(Options);

	// Write struct
	ConfigWriterType Writer;
	Writer.SetLogVerbose(true);
	Writer.Write(StructType::StaticStruct(), &ExportedStruct);
	const FString WriterString = Writer.GetAsString();

	// Read strusct
	ConfigParserType Parser;
	Parser.SetLogVerbose(true);
	Parser.InitializeParserFromString(WriterString);
	Parser.ReadAllProperty(StructType::StaticStruct(), &ImportedStruct);

	// Should be the same
	FString ErrorMessage;
	if (ExportedStruct.IsEqual(ImportedStruct, ErrorMessage))
	{
		return true;
	}

	if (NameWriterType.IsEmpty() || NameParserType.IsEmpty())
	{
		UE_LOG(LogDlgIOTester, Warning, TEXT("TestStruct: Test Failed: %s"), *StructDescription);
	}
	else
	{
		// Used only for debugging
		ConfigWriterType DebugParser;
		DebugParser.Write(StructType::StaticStruct(), &ImportedStruct);
		const FString ParserString = DebugParser.GetAsString();

		UE_LOG(LogDlgIOTester, Warning, TEXT("This = ExportedStruct, Other = ImportedStruct"));
		UE_LOG(LogDlgIOTester, Warning, TEXT("Writer = %s, Parser = %s. Test Failed = %s"), *NameWriterType, *NameParserType, *StructDescription);
		UE_LOG(LogDlgIOTester, Warning, TEXT("ErrorMessage = %s"), *ErrorMessage);
		UE_LOG(LogDlgIOTester, Warning, TEXT("ExportedStruct.GetAsString() = |%s|\n"), *WriterString);
		UE_LOG(LogDlgIOTester, Warning, TEXT("ImportedStruct.GetAsString() = |%s|\n"), *ParserString);
		UE_LOG(LogDlgIOTester, Warning, TEXT(""));
	}

	return false;
}
