// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

// The name of the Dialogue System plugin as defined in the .uplugin file
const FName DIALOGUE_SYSTEM_PLUGIN_NAME(TEXT("DlgSystem"));

class AActor;
class SWidget;
class SDockTab;
struct FTabSpawnerEntry;

/**
 * Interface for the DlgSystem module.
 */
class DLGSYSTEM_API IDlgSystemModule : public IModuleInterface
{
public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static IDlgSystemModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IDlgSystemModule>(DIALOGUE_SYSTEM_PLUGIN_NAME);
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(DIALOGUE_SYSTEM_PLUGIN_NAME);
	}

public:
	virtual ~IDlgSystemModule() {}

	/**
	 * Registers all the console commands.
	 * @param WorldConextObjectPtr - The reference actor for the World. Without this the runtime module won't know how to get the UWorld.
	 */
	virtual void RegisterConsoleCommands(const TWeakObjectPtr<const UObject>& WorldContextObjectPtr) = 0;

	// Unregister all the console commands
	virtual void UnregisterConsoleCommands() = 0;

	// Gets the debug Dialogue Data Display Window.
	virtual TSharedRef<SWidget> GetDialogueDataDisplayWindow() = 0;

	// Gets the tab entry for the Window
	virtual FTabSpawnerEntry* GetDialogueDataDisplaySpawnEntry() = 0;

	// Display the debug Dialogue Data Window on the screen
	virtual void DisplayDialogueDataWindow() = 0;
};
