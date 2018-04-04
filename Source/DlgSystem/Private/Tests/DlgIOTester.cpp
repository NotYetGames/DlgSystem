// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgIOTester.h"

#include "FileHelper.h"
#include "EnumProperty.h"
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
	bAllSucceeded = TestParser<FDlgConfigWriter, FDlgConfigParser>(Options, TEXT("FDlgConfigWriter"), TEXT("FDlgConfigParser"))
		&& bAllSucceeded;

	Options = {};
	bAllSucceeded = TestParser<FDlgJsonWriter, FDlgJsonParser>(Options, TEXT("FDlgJsonWriter"), TEXT("FDlgJsonParser"))
		&& bAllSucceeded;

	return bAllSucceeded;
}

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDlgIOAutomationTest, "DlgSystem.IO.Tests", EAutomationTestFlags::Disabled | EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FDlgIOAutomationTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("true == true"), true, true);
	TestTrue(TEXT("Testing all parsers"), FDlgIOTester::TestAllParsers());

	return true;
}

#endif //WITH_DEV_AUTOMATION_TESTS
