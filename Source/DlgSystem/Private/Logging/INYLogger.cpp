// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#include "Logging/INYLogger.h"
#include "CoreGlobals.h"
#include "Misc/OutputDevice.h"
#include "Misc/UObjectToken.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Logging/MessageLog.h"

#if WITH_UNREAL_DEVELOPER_TOOLS
#include "MessageLogInitializationOptions.h"
#include "MessageLogModule.h"
#endif // WITH_UNREAL_DEVELOPER_TOOLS

#include "Modules/ModuleManager.h"

INYLogger& INYLogger::SetClientConsolePlayerController(APlayerController* PC)
{
	PlayerController = PC;
	return *this;
}

FOutputDevice* INYLogger::GetOutputDeviceFromLogLevel(ENYLoggerLogLevel Level)
{
#if NO_LOGGING
	return nullptr;
#endif

	switch (Level)
	{
	case ENYLoggerLogLevel::NoLogging:
		return nullptr;

	case ENYLoggerLogLevel::Error:
	case ENYLoggerLogLevel::Warning:
		return GWarn;

	default:
		return GLog;
	}
}

#if WITH_UNREAL_DEVELOPER_TOOLS
FMessageLogModule* INYLogger::GetMessageLogModule()
{
	return FModuleManager::LoadModulePtr<FMessageLogModule>("MessageLog");
}
#endif // WITH_UNREAL_DEVELOPER_TOOLS

bool INYLogger::IsMessageLogNameRegistered(FName LogName)
{
#if WITH_UNREAL_DEVELOPER_TOOLS
	FMessageLogModule* MessageLogModule = GetMessageLogModule();
	if (!MessageLogModule)
	{
		return false;
	}

	return MessageLogModule->IsRegisteredLogListing(LogName);
#else
	return false;	
#endif // WITH_UNREAL_DEVELOPER_TOOLS
}

void INYLogger::MessageLogRegisterLogName(FName LogName, const FText& LogLabel)
{
#if WITH_UNREAL_DEVELOPER_TOOLS
	FMessageLogModule* MessageLogModule = GetMessageLogModule();
	if (!MessageLogModule)
	{
		return;
	}

	FMessageLogInitializationOptions InitOptions;
	MessageLogModule->RegisterLogListing(LogName, LogLabel, InitOptions);
#endif // WITH_UNREAL_DEVELOPER_TOOLS
}

bool INYLogger::MessageLogUnregisterLogName(FName LogName)
{
#if WITH_UNREAL_DEVELOPER_TOOLS
	FMessageLogModule* MessageLogModule = GetMessageLogModule();
	if (!MessageLogModule)
	{
		return false;
	}

	return MessageLogModule->UnregisterLogListing(LogName);
#else
	return false;
#endif // WITH_UNREAL_DEVELOPER_TOOLS
}

#if WITH_UNREAL_DEVELOPER_TOOLS
TSharedPtr<IMessageLogListing> INYLogger::MessageLogGetLogNameListing(FName LogName)
{
	FMessageLogModule* MessageLogModule = GetMessageLogModule();
	if (!MessageLogModule)
	{
		return nullptr;
	}

	return MessageLogModule->GetLogListing(LogName);
}
#endif // WITH_UNREAL_DEVELOPER_TOOLS

void INYLogger::MessageLogOpenLogName(FName LogName)
{
#if WITH_UNREAL_DEVELOPER_TOOLS
	FMessageLogModule* MessageLogModule = GetMessageLogModule();
	if (!MessageLogModule)
	{
		return;
	}

	MessageLogModule->OpenMessageLog(LogName);
#endif // WITH_UNREAL_DEVELOPER_TOOLS
}

void INYLogger::Log(ENYLoggerLogLevel Level, const FString& Message)
{
	// No logging, abort
#if NO_LOGGING
	return;
#endif

	if (UseClientConsole())
	{
		LogClientConsole(Level, Message);
	}
	if (UseOnScreen())
	{
		LogScreen(Level, Message);
	}
	if (UseOutputLog())
	{
		LogOutputLog(Level, Message);
	}
	if (UseMessageLog())
	{
		LogMessageLog(Level, Message);
	}
}

void INYLogger::LogScreen(ENYLoggerLogLevel Level, const FString& Message)
{
	if (!GEngine)
	{
		return;
	}

	const bool bPreviousValue = GAreScreenMessagesEnabled;
	if (bForceEnableScreenMessages)
	{
		GAreScreenMessagesEnabled = true;
	}

	const uint64 Key = INDEX_NONE;
	const FColor Color = GetColorForLogLevel(Level);
	GEngine->AddOnScreenDebugMessage(Key, ScreenLogDisplayTimeSeconds, Color, Message, bScreenNewerOnTop, ScreenTextScale);

	if (bForceEnableScreenMessages)
	{
		GAreScreenMessagesEnabled = bPreviousValue;
	}
}

void INYLogger::LogClientConsole(ENYLoggerLogLevel Level, const FString& Message)
{
	if (!IsValid(PlayerController) || !PlayerController->IsValidLowLevelFast())
	{
		return;
	}

	// These arguments seem to not be used
	const FName Type = NAME_None;
	const float LifetimeSeconds = 0.f;
	PlayerController->ClientMessage(Message, Type, LifetimeSeconds);
}

void INYLogger::LogMessageLog(ENYLoggerLogLevel Level, const FString& Message)
{
	// TSharedRef<FTokenizedMessage> NewMessage = FTokenizedMessage::Create(Severity);
	const EMessageSeverity::Type Severity = GetMessageSeverityForLogLevel(Level);
	auto message = FMessageLog("Dialogue Plugin");
	message.SuppressLoggingToOutputLog(bMessageLogSuppressLoggingToOutputLog);
	message.Message(Severity, FText::FromString(Message));

	if (bMessageLogOpen)
	{
		message.Open(Severity, true);
	}
}

void INYLogger::LogOutputLog(ENYLoggerLogLevel Level, const FString& Message)
{
	FOutputDevice* LogDevice = GetOutputDeviceFromLogLevel(Level);
	if (!LogDevice)
	{
		return;
	}

	const ELogVerbosity::Type UnrealLogType = GetUnrealLogTypeForLogLevel(Level);
	LogDevice->Log(OutputLogCategory, UnrealLogType, Message);
}


void INYLogger::ClearAllOnScreenLogs()
{
	if (!GEngine)
	{
		return;
	}

	GEngine->ClearOnScreenDebugMessages();
}
