// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#include "Logging/DlgLogger.h"

#define LOCTEXT_NAMESPACE "DlgLogger"

static const FName NAME{TEXT("Dialogue Plugin")};

void FDlgLogger::OnStart()
{
	MessageLogRegisterLogName(NAME, LOCTEXT("dlg_key", "Dialogue Plugin"));
}

void FDlgLogger::OnShutdown()
{
	MessageLogUnregisterLogName(NAME);
}

#undef  LOCTEXT_NAMESPACE
