// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "Tests/DlgIOTester.h"

#include "CoreTypes.h"
#include "Containers/UnrealString.h"
#include "Misc/AutomationTest.h"

#include "IO/DlgConfigWriter.h"
#include "IO/DlgConfigParser.h"
#include "IO/DlgJsonParser.h"
#include "IO/DlgJsonWriter.h"

DEFINE_LOG_CATEGORY(LogDlgIOTester);

bool FDlgIOTester::TestAllParsers()
{
	bool bAllSucceeded = true;

	FDlgIOTesterOptions Options;
	Options.bSupportsPureEnumContainer = false;
	Options.bSupportsNonPrimitiveInSet = false;
	Options.bSupportsColorPrimitives = false;
	Options.bSupportsDatePrimitive = false;
	Options.bSupportsUObjectValueInMap = false;
	bAllSucceeded &= TestParser<FDlgConfigWriter, FDlgConfigParser>(Options, TEXT("FDlgConfigWriter"), TEXT("FDlgConfigParser"));

	Options = {};
	Options.bSupportsDatePrimitive = false;
	Options.bSupportsUObjectValueInMap = false;
	bAllSucceeded &= TestParser<FDlgJsonWriter, FDlgJsonParser>(Options, TEXT("FDlgJsonWriter"), TEXT("FDlgJsonParser"));

	return bAllSucceeded;
}

#if WITH_DEV_AUTOMATION_TESTS

// NOTE: to run this test, first remove the EAutomationTestFlags::Disabled flag
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDlgIOAutomationTest, "DlgSystem.IO.Tests", //EAutomationTestFlags::Disabled |
																			 EAutomationTestFlags::RequiresUser |
																			 EAutomationTestFlags::EditorContext |
																			 EAutomationTestFlags::ClientContext |
																			 EAutomationTestFlags::CommandletContext |
																			 EAutomationTestFlags::ProductFilter)

bool FDlgIOAutomationTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("true == true"), true, true);
	TestTrue(TEXT("Testing all parsers"), FDlgIOTester::TestAllParsers());

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS
