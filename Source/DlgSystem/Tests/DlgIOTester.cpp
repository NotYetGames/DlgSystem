// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.

#include "CoreTypes.h"
#include "DlgIOTesterTypes.h"
#include "Containers/UnrealString.h"
#include "Misc/AutomationTest.h"

#include "DlgSystem/IO/DlgConfigWriter.h"
#include "DlgSystem/IO/DlgConfigParser.h"
#include "DlgSystem/IO/DlgJsonParser.h"
#include "DlgSystem/IO/DlgJsonWriter.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgIOTester, All, All);
DEFINE_LOG_CATEGORY(LogDlgIOTester);

#if WITH_DEV_AUTOMATION_TESTS

class FDlgIOTester
{
public:
	// Tests Parser and Writer
	template <typename ConfigWriterType, typename ConfigParserType>
	static bool TestParser(
			FAutomationTestBase& Test,
			const FDlgIOTesterOptions& Options,
			const FString NameWriterType = FString(),
			const FString NameParserType = FString()
		);

	// Test all parsers/writers
	static bool TestAllParsers(FAutomationTestBase& Test);

	template <typename ConfigWriterType, typename ConfigParserType, typename StructType>
	static bool TestStruct(
		FAutomationTestBase& Test,
		const FString& StructDescription,
		const FDlgIOTesterOptions& Options,
		const FString NameWriterType = FString(),
		const FString NameParserType = FString()
	);
};


template <typename ConfigWriterType, typename ConfigParserType>
bool FDlgIOTester::TestParser(
	FAutomationTestBase& Test,
	const FDlgIOTesterOptions& Options,
	const FString NameWriterType,
	const FString NameParserType
)
{
	bool bAllSucceeded = true;

	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestStructPrimitives>(Test, "Struct of Primitives", Options, NameWriterType, NameParserType);
	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestStructComplex>(Test, "Struct of Complex types", Options, NameWriterType, NameParserType);

	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestArrayPrimitive>(Test, "Array of Primitives", Options, NameWriterType, NameParserType);
	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestArrayComplex>(Test, "Array of Complex types", Options, NameWriterType, NameParserType);

	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestSetPrimitive>(Test, "Set of Primitives", Options, NameWriterType, NameParserType);
	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestSetComplex>(Test, "Set of Complex types", Options, NameWriterType, NameParserType);

	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestMapPrimitive>(Test, "Map with Primitives", Options, NameWriterType, NameParserType);
	bAllSucceeded &= TestStruct<ConfigWriterType, ConfigParserType, FDlgTestMapComplex>(Test, "Map with Complex types", Options, NameWriterType, NameParserType);

	return bAllSucceeded;
}


template <typename ConfigWriterType, typename ConfigParserType, typename StructType>
bool FDlgIOTester::TestStruct(
	FAutomationTestBase& Test,
	const FString& StructDescription,
	const FDlgIOTesterOptions& Options,
	const FString NameWriterType,
	const FString NameParserType
)
{
	StructType ExportedStruct;
	StructType ImportedStruct;
	ExportedStruct.GenerateRandomData(Options);
	ImportedStruct.GenerateRandomData(Options);

	// Write struct
	ConfigWriterType Writer;
	//Writer.SetLogVerbose(true);
	Writer.Write(StructType::StaticStruct(), &ExportedStruct);
	const FString WriterString = Writer.GetAsString();

	// Read struct
	ConfigParserType Parser;
	//Parser.SetLogVerbose(true);
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
		UE_LOG(LogDlgIOTester, Warning, TEXT("TestStruct: Test Failed (both empty) = %s"), *StructDescription);
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

bool FDlgIOTester::TestAllParsers(FAutomationTestBase& Test)
{
	bool bAllSucceeded = true;

	FDlgIOTesterOptions Options;
	Options.bSupportsDatePrimitive = false;
	Options.bSupportsUObjectValueInMap = false;
	bAllSucceeded &= TestParser<FDlgJsonWriter, FDlgJsonParser>(Test, Options, TEXT("FDlgJsonWriter"), TEXT("FDlgJsonParser"));

	Options = {};
	Options.bSupportsPureEnumContainer = false;
	Options.bSupportsNonPrimitiveInSet = false;
	Options.bSupportsColorPrimitives = false;
	Options.bSupportsDatePrimitive = false;
	Options.bSupportsUObjectValueInMap = false;
	bAllSucceeded &= TestParser<FDlgConfigWriter, FDlgConfigParser>(Test, Options, TEXT("FDlgConfigWriter"), TEXT("FDlgConfigParser"));

	return bAllSucceeded;
}

// NOTE: to run this test, first remove the EAutomationTestFlags::Disabled flag
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDlgIOAutomationTest,
	"DlgSystem.IO.Tests",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
	//EAutomationTestFlags::Disabled |
	// EAutomationTestFlags::RequiresUser |
	// EAutomationTestFlags::ApplicationContextMask |
	// EAutomationTestFlags::ProductFilter)

bool FDlgIOAutomationTest::RunTest(const FString& Parameters)
{
	// UE_LOG(LogTemp, Error, TEXT("FDlgIOAutomationTest::RunTest"));
	TestEqual(TEXT("true == true"), true, true);
	TestTrue(TEXT("Testing all parsers"), FDlgIOTester::TestAllParsers(*this));

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS
