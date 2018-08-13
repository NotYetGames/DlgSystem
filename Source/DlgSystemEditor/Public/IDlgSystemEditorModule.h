// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

// The name of the Dialogue System Editor plugin as defined in the .uplugin file
const FName DIALOGUE_SYSTEM_EDITOR_PLUGIN_NAME(TEXT("DlgSystemEditor"));

/**
 * Interface for the DlgSystemEditor module.
 */
class DLGSYSTEMEDITOR_API IDlgSystemEditorModule : public IModuleInterface
{
public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IDlgSystemEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IDlgSystemEditorModule>(DIALOGUE_SYSTEM_EDITOR_PLUGIN_NAME);
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(DIALOGUE_SYSTEM_EDITOR_PLUGIN_NAME);
	}

public:
	virtual ~IDlgSystemEditorModule() {}
};
