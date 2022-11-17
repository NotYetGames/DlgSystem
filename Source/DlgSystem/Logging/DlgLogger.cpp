// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgLogger.h"
#include "DlgSystem/DlgSystemModule.h"
#include "DlgSystem/DlgSystemSettings.h"

#define LOCTEXT_NAMESPACE "DlgLogger"

static const FName MESSAGE_LOG_NAME{TEXT("Dialogue Plugin")};

FDlgLogger::FDlgLogger() : Super()
{
	static constexpr bool bOwnMessageLogMirrorToOutputLog = true;
	EnableMessageLog(bOwnMessageLogMirrorToOutputLog);
	SetMessageLogMirrorToOutputLog(true);

	DisableOutputLog();
	DisableOnScreen();
	DisableClientConsole();

	// We mirror everything to the output log so that is why we disabled the output log above
	SetOutputLogCategory(LogDlgSystem);
	SetMessageLogName(MESSAGE_LOG_NAME, false);
	SetMessageLogOpenOnNewMessage(true);
	SetRedirectMessageLogLevelsHigherThan(ENYLoggerLogLevel::Warning);
	SetOpenMessageLogLevelsHigherThan(ENYLoggerLogLevel::NoLogging);
}

FDlgLogger& FDlgLogger::SyncWithSettings()
{
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();

	UseMessageLog(Settings->bEnableMessageLog);
	SetMessageLogMirrorToOutputLog(Settings->bMessageLogMirrorToOutputLog);
	UseOutputLog(Settings->bEnableOutputLog);
	SetRedirectMessageLogLevelsHigherThan(Settings->RedirectMessageLogLevelsHigherThan);
	SetOpenMessageLogLevelsHigherThan(Settings->OpenMessageLogLevelsHigherThan);
	SetMessageLogOpenOnNewMessage(Settings->bMessageLogOpen);

	return *this;
}

void FDlgLogger::OnStart()
{
	MessageLogRegisterLogName(MESSAGE_LOG_NAME, LOCTEXT("dlg_key", "Dialogue System Plugin"));
	Get().SyncWithSettings();
}

void FDlgLogger::OnShutdown()
{
	MessageLogUnregisterLogName(MESSAGE_LOG_NAME);
}

#undef  LOCTEXT_NAMESPACE
