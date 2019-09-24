// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "INYLogger.h"


class DLGSYSTEM_API FDlgLogger : public INYLogger
{
	typedef FDlgLogger Self;
public:
	FDlgLogger()
	{
		bOnScreen = true;
		bClientConsole = true;
		bOutputLog = true;
		bMessageLog = true;
	}
	
	virtual ~FDlgLogger() {}

	// Create a new logger
	static FDlgLogger New() { return Self{}; }

	static void OnStart();
	static void OnShutdown();
};
