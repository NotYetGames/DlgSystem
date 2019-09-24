// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/LogCategory.h"
// #include "INYLogger.generated.h"

class FOutputDevice;
class APlayerController;
class FMessageLogModule;
class IMessageLogListing;


UENUM()
enum class ENYLoggerLogLevel : uint8
{
	NoLogging = 0,

	// Kills the program
	// TODO use
	Fatal,

	Error,
	Warning,

	// Log
	Info,
	//Log = Info,

	// Verbose
	Debug,
	//Verbose = Debug,

	// VeryVerbose
	// Used for
	Trace,
	//VeryVerbose = Trace
};


/**
 * The following output are available:
 * - output log, also file on your filesystem which corresponds to the output log
 * - message log, only available in editor
 * - in the client console, press ~ (tilde) to see it
 * - on screen
 */
class DLGSYSTEM_API INYLogger
{
	typedef INYLogger Self;
public:
	INYLogger() {}
	virtual ~INYLogger() {}

	// Create a new logger
	static INYLogger New() { return Self{}; }

	//
	// Client console
	//
	
	Self& EnableClientConsole(APlayerController* PC)
	{
		SetUseClientConsole(true);
		SetClientConsolePlayerController(PC);
		return *this;
	}
	Self& DisableClientConsole() { return SetUseClientConsole(false); }
	Self& SetUseClientConsole(bool bValue)
	{
		bClientConsole = bValue;
		return *this;
	}
	Self& SetClientConsolePlayerController(APlayerController* PC);

	//
	// On screen
	//
	
	// bInForceEnableScreenMessages - if true, even if the screen messages are disabled we will force display it
	Self& EnableOnScreen(bool bInForceEnableScreenMessages = false) { return SetUseOnScreen(true, bInForceEnableScreenMessages); }
	Self& DisableOnScreen() { return SetUseOnScreen(false); }
	Self& SetUseOnScreen(bool bValue, bool bInForceEnableScreenMessages = false)
	{
		bOnScreen = bValue;
		bForceEnableScreenMessages = bInForceEnableScreenMessages;
		return *this;
	}

	// How long to display the on screen log messages
	Self& SetOnScreenTimeToDisplay(float Seconds)
	{
		ScreenLogDisplayTimeSeconds = Seconds;
		return *this;
	}

	// Should newer messages appear on top
	Self& SetOnScreenNewerOnTop(bool bValue)
	{
		bScreenNewerOnTop = bValue;
		return *this;
	}

	// Clears all the on screen messages
	static void ClearAllOnScreenLogs();

	//
	// Output log
	//
	
	Self& EnableOutputLog() { return SetUseOutputLog(true); }
	Self& DisableOutputLog() { return SetUseOutputLog(false); }
	Self& SetUseOutputLog(bool bValue)
	{
		bOutputLog = bValue;
		return *this;
	}
	
	// The log category must exist
	Self& SetOutputLogCategory(const FLogCategoryBase& NewCategory)
	{
		OutputLogCategory = NewCategory.GetCategoryName();
		return *this;
	}

	Self& SetOutputLogCategory(FName NewCategory)
	{
		OutputLogCategory = NewCategory;
		return *this;
	}

	//
	// Message log
	//
	
	Self& EnableMessageLog() { return SetUseMessageLog(true); }
	Self& DisableMessageLog() { return SetUseMessageLog(true); }
	Self& SetUseMessageLog(bool bValue)
	{
		bMessageLog = bValue;
		return *this;
	}

	// Opens the log for display to the user given certain conditions.
	Self& SetMessageLogOpenOnNewMessage(bool bValue)
	{
		bMessageLogOpen = bValue;
		return *this;
	}

	// Should we mirror message log messages from this instance to the output log during flush?
	Self& SetMessageLogSuppressLoggingToOutputLog(bool bValue)
	{
		bMessageLogSuppressLoggingToOutputLog = bValue;
		return *this;
	}

	static bool IsMessageLogNameRegistered(FName LogName);
	static bool MessageLogUnregisterLogName(FName LogName);
	static void MessageLogRegisterLogName(FName LogName, const FText& LogLabel);
#if WITH_UNREAL_DEVELOPER_TOOLS
	static TSharedPtr<IMessageLogListing> MessageLogGetLogNameListing(FName LogName);
#endif // WITH_UNREAL_DEVELOPER_TOOLS
	static void MessageLogOpenLogName(FName LogName);
	

	// Registers the new Message log name
	// NOTE: Call MessageLogRegisterLogName before calling this
	Self& SetMessageLogName(FName LogName)
	{
#if WITH_UNREAL_DEVELOPER_TOOLS
		if (!IsMessageLogNameRegistered(LogName))
		{
			Warning(TEXT("SetMessageLogName: Failed to register the message log name"));
		}
#endif // WITH_UNREAL_DEVELOPER_TOOLS
		
		MessageLogName = LogName;
		return *this;
	}

	//
	// Public accessors
	//

	FORCEINLINE FName GetOutputLogCategory() const { return OutputLogCategory; }
	FORCEINLINE bool UseClientConsole() const { return bClientConsole; }
	FORCEINLINE bool UseOnScreen() const { return bOnScreen; }
	FORCEINLINE bool UseOutputLog() const { return bOutputLog; }
	FORCEINLINE bool UseMessageLog() const { return bMessageLog; }

	void Log(ENYLoggerLogLevel Level, const FString& Message);

	// TODO implement
	// void Fatal(const FString& Message) { Log(ENYLoggerLogLevel::Fatal, Message); }
	FORCEINLINE void Error(const FString& Message) { Log(ENYLoggerLogLevel::Error, Message); }
	FORCEINLINE void Warning(const FString& Message) { Log(ENYLoggerLogLevel::Warning, Message); }
	FORCEINLINE void Info(const FString& Message) { Log(ENYLoggerLogLevel::Info, Message); }
	FORCEINLINE void Debug(const FString& Message) { Log(ENYLoggerLogLevel::Debug, Message); }
	FORCEINLINE void Trace(const FString& Message) { Log(ENYLoggerLogLevel::Trace, Message); }

protected:
#if WITH_UNREAL_DEVELOPER_TOOLS
	static FMessageLogModule* GetMessageLogModule();
#endif // WITH_UNREAL_DEVELOPER_TOOLS

	
	virtual void LogScreen(ENYLoggerLogLevel Level, const FString& Message);
	virtual void LogOutputLog(ENYLoggerLogLevel Level, const FString& Message);
	virtual void LogMessageLog(ENYLoggerLogLevel Level, const FString& Message);
	virtual void LogClientConsole(ENYLoggerLogLevel Level, const FString& Message);

	static ELogVerbosity::Type GetUnrealLogTypeForLogLevel(ENYLoggerLogLevel Level)
	{
	 	switch (Level)
		{
		case ENYLoggerLogLevel::Fatal:
			return ELogVerbosity::Fatal;

		case ENYLoggerLogLevel::Error:
			return ELogVerbosity::Error;

		case ENYLoggerLogLevel::Warning:
			return ELogVerbosity::Warning;

		case ENYLoggerLogLevel::Debug:
			return ELogVerbosity::Verbose;

		case ENYLoggerLogLevel::Trace:
			return ELogVerbosity::VeryVerbose;

		default:
			return ELogVerbosity::Log;
		}
	}
	static EMessageSeverity::Type GetMessageSeverityForLogLevel(ENYLoggerLogLevel Level)
	{
		switch (Level)
		{
		case ENYLoggerLogLevel::Fatal:
			return EMessageSeverity::CriticalError;

		case ENYLoggerLogLevel::Error:
			return EMessageSeverity::Error;

		case ENYLoggerLogLevel::Warning:
			return EMessageSeverity::Warning;

		default:
			return EMessageSeverity::Info;
		}
	}

	FColor GetColorForLogLevel(ENYLoggerLogLevel Level) const
	{
		switch (Level)
		{
		case ENYLoggerLogLevel::Fatal:
			return ColorFatal;

		case ENYLoggerLogLevel::Error:
			return ColorError;

		case ENYLoggerLogLevel::Warning:
			return ColorWarning;

		case ENYLoggerLogLevel::Debug:
			return ColorDebug;

		case ENYLoggerLogLevel::Trace:
			return ColorTrace;

		default:
			return ColorInfo;
		}
	}
	static FOutputDevice* GetOutputDeviceFromLogLevel(ENYLoggerLogLevel Level);

protected:
	// If set to false, the class will use GLog/GWarn to log to the log file
	// Instead of the own implementation of the category
	// bool bUseLogCategory = false;
	//
	
	// Output to the screen
	bool bOnScreen = false;
	
	// Time to stay on screen
	float ScreenLogDisplayTimeSeconds = 5.f;

	// How to scale the text for the on screen messages
	FVector2D ScreenTextScale = FVector2D::UnitVector;

	// Newer screen messages appear on top
	bool bScreenNewerOnTop = true;

	// If bScreen == true and this is true, we force enable the screen messages
	bool bForceEnableScreenMessages = false;
	
	
	// Output to the output log and log file
	bool bOutputLog = true;

	// Category for output l og
	FName OutputLogCategory = TEXT("LogTemp");

	
	// Output to the message log
	bool bMessageLog = true;

	// Category for message log
	FName MessageLogName = TEXT("PIE");

	// Should we mirror message log messages from this instance to the output log during flush?
	bool bMessageLogSuppressLoggingToOutputLog = false;

	// Opens the log for display to the user given certain conditions.
	bool bMessageLogOpen = true;
	

	// Output to the dropdown ingame console, requires the PlayerController to be set
	bool bClientConsole = false;

	// Required to print to client console
	APlayerController* PlayerController = nullptr;

	// Colors
	FColor ColorFatal = FColor::Red;
	FColor ColorError = FColor::Red;
	FColor ColorWarning = FColor::Yellow;
	FColor ColorInfo = FColor::White;
	FColor ColorDebug = FColor::Blue;
	FColor ColorTrace = FColor::Cyan;
};
