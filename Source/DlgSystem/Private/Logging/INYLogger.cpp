// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "Logging/INYLogger.h"
#include "CoreGlobals.h"
#include "Misc/OutputDevice.h"
#include "Misc/UObjectToken.h"
#include "Misc/FeedbackContext.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Logging/MessageLog.h"

#if WITH_UNREAL_DEVELOPER_TOOLS
#include "MessageLogInitializationOptions.h"
#include "MessageLogModule.h"
#endif // WITH_UNREAL_DEVELOPER_TOOLS

#include "Modules/ModuleManager.h"

#if ENGINE_MINOR_VERSION >= 24
	#define NY_ARRAY_COUNT UE_ARRAY_COUNT
#else
	#define NY_ARRAY_COUNT ARRAY_COUNT
#endif


// Pulled the two FOutputDevice::Logf functions into shared code. Needs to be a #define
// since it uses GET_VARARGS_RESULT which uses the va_list stuff which operates on the
// current function, so we can't easily call a function
#define NY_GROWABLE_LOGF(SerializeFunc) \
	int32	BufferSize	= 1024; \
	TCHAR*	Buffer		= NULL; \
	int32	Result		= -1; \
	/* allocate some stack space to use on the first pass, which matches most strings */ \
	TCHAR	StackBuffer[512]; \
	TCHAR*	AllocatedBuffer = NULL; \
\
	/* first, try using the stack buffer */ \
	Buffer = StackBuffer; \
	GET_VARARGS_RESULT( Buffer, NY_ARRAY_COUNT(StackBuffer), NY_ARRAY_COUNT(StackBuffer) - 1, Fmt, Fmt, Result ); \
\
	/* if that fails, then use heap allocation to make enough space */ \
	while(Result == -1) \
	{ \
		FMemory::SystemFree(AllocatedBuffer); \
		/* We need to use malloc here directly as GMalloc might not be safe. */ \
		Buffer = AllocatedBuffer = (TCHAR*) FMemory::SystemMalloc( BufferSize * sizeof(TCHAR) ); \
		GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize-1, Fmt, Fmt, Result ); \
		BufferSize *= 2; \
	}; \
	Buffer[Result] = 0; \
	; \
\
	SerializeFunc; \
	FMemory::SystemFree(AllocatedBuffer);


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

void INYLogger::MessageLogRegisterLogName(FName LogName, const FText& LogLabel, const FNYMessageLogInitializationOptions& InitOptions)
{
#if WITH_UNREAL_DEVELOPER_TOOLS
	FMessageLogModule* MessageLogModule = GetMessageLogModule();
	if (!MessageLogModule)
	{
		return;
	}

	FMessageLogInitializationOptions UnrealInitOptions;
	UnrealInitOptions.bShowFilters = InitOptions.bShowFilters;
	UnrealInitOptions.bShowPages = InitOptions.bShowPages;
	UnrealInitOptions.bAllowClear = InitOptions.bAllowClear;
	UnrealInitOptions.bDiscardDuplicates = InitOptions.bDiscardDuplicates;
	UnrealInitOptions.MaxPageCount = InitOptions.MaxPageCount;
	UnrealInitOptions.bShowInLogWindow = InitOptions.bShowInLogWindow;

	MessageLogModule->RegisterLogListing(LogName, LogLabel, UnrealInitOptions);
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

void INYLogger::LogfImplementation(ENYLoggerLogLevel Level, const TCHAR* Fmt, ...)
{
#if !NO_LOGGING
	NY_GROWABLE_LOGF(Log(Level, Buffer))
#endif // !NO_LOGGING
}

// void INYLogger::Fatal(const ANSICHAR* File, int32 Line, const FString& Message)
// {
// #if NO_LOGGING
// 	LowLevelFatalErrorHandler(File, Line, *Message);
// 	_DebugBreakAndPromptForRemote();
// 	FDebug::AssertFailed("", File, Line, *Message);
// #else
// 	LowLevelFatalErrorHandler(File, Line, *Message);
// 	_DebugBreakAndPromptForRemote();
// 	FDebug::AssertFailed("", File, Line, *Message);
// #endif // NO_LOGGING
// }

void INYLogger::Log(ENYLoggerLogLevel Level, const FString& Message)
{
	// Should not happen but just in case redirect to the fatal function
	// if (Level == ENYLoggerLogLevel::Fatal)
	// {
	// 	Fatal(__FILE__, __LINE__, Message);
	// 	return;
	// }

	// No logging, abort
#if !NO_LOGGING
	if (IsClientConsoleEnabled())
	{
		LogClientConsole(Level, Message);
	}
	if (IsOnScreenEnabled())
	{
		LogScreen(Level, Message);
	}
	if (IsOutputLogEnabled())
	{
		LogOutputLog(Level, Message);
	}
	if (IsMessageLogEnabled())
	{
		LogMessageLog(Level, Message);
	}
#endif // !NO_LOGGING
}

void INYLogger::LogScreen(ENYLoggerLogLevel Level, const FString& Message)
{
	if (!GEngine)
	{
		return;
	}

	const bool bPreviousValue = AreAllOnScreenMessagesEnabled();
	if (bForceEnableScreenMessages)
	{
		EnableAllOnScreenMessages();
	}

	const uint64 Key = INDEX_NONE;
	const FColor Color = GetColorForLogLevel(Level);
	GEngine->AddOnScreenDebugMessage(Key, ScreenLogDisplayTimeSeconds, Color, Message, bScreenNewerOnTop, ScreenTextScale);

	if (bForceEnableScreenMessages)
	{
		SetAreAllOnScreenMessagesEnabled(bPreviousValue);
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
	// Should we be redirecting this message log because
	if (RedirectMessageLogLevelsHigherThan != ENYLoggerLogLevel::NoLogging &&
		Level > RedirectMessageLogLevelsHigherThan)
	{
		// Redirect to the output log if not enabled
		if (!IsOutputLogEnabled())
		{
			LogOutputLog(Level, Message);
		}
		return;
	}

	// TSharedRef<FTokenizedMessage> NewMessage = FTokenizedMessage::Create(Severity);
	const EMessageSeverity::Type Severity = GetMessageSeverityForLogLevel(Level);
	auto MessageLog = FMessageLog(MessageLogName);
	MessageLog.SuppressLoggingToOutputLog(!bMessageLogMirrorToOutputLog);
	MessageLog.Message(Severity, FText::FromString(Message));

	// Open message log
	if (bMessageLogOpen && Level > OpenMessageLogLevelsHigherThan)
	{
		MessageLog.Open(Severity, false);
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

#undef NY_ARRAY_COUNT
