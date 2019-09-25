// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#include "Logging/DlgLogger.h"
#include "DlgSystemPrivatePCH.h"

#define LOCTEXT_NAMESPACE "DlgLogger"

static const FName MESSAGE_LOG_NAME{TEXT("Dialogue Plugin")};

FDlgLogger::FDlgLogger() : Super()
{
	EnableMessageLog(false);
	DisableOutputLog();
	DisableOnScreen();
	DisableClientConsole();
	
	SetOutputLogCategory(LogDlgSystem);
	SetMessageLogName(MESSAGE_LOG_NAME, false);
	SetRedirectMessageLogLevelsHigherThan(ENYLoggerLogLevel::Warning);
}

void FDlgLogger::OnStart()
{
	MessageLogRegisterLogName(MESSAGE_LOG_NAME, LOCTEXT("dlg_key", "Dialogue System Plugin"));
}

void FDlgLogger::OnShutdown()
{
	MessageLogUnregisterLogName(MESSAGE_LOG_NAME);
}

#undef  LOCTEXT_NAMESPACE
