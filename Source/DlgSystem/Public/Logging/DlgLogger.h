// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "INYLogger.h"


class DLGSYSTEM_API FDlgLogger : public INYLogger
{
	typedef FDlgLogger Self;
	typedef INYLogger Super;

protected:
	FDlgLogger();
	
public:
	virtual ~FDlgLogger() {}

	// Sync values with system UDlgSystemSettings values
	Self& SyncWithSettings();
	
	// Create a new logger
	static FDlgLogger New() { return Self{}; }
	static FDlgLogger& Get()
	{
		static FDlgLogger Instance;
		return Instance;
	}

	static void OnStart();
	static void OnShutdown();
};
