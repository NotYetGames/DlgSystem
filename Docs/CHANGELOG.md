# 9.1

## Important Bug Fixes

- **Fixed** Custom Condition when set on edges in packaged build. **TO FIX THEM** in your project you have to reset them on all edges where they are used.

- **Fixed** Edges not showing sometimes when creating a new node. See commit [671db137](https://gitlab.com/NotYetGames/DlgSystem/-/commit/671db137dd8190127f4c885f8fb795368ba56810) for more details

## Other Changes

- **Improved** error messages context and text so that it is easier now to know from where the error comes.

- **Improve** fixed some false positive warning messages when using condition types that did not require a participant

- **Improvement** Make everything more blueprint friendly, everything that is dialogue data related should be read accessible from Blueprints

- **Removed** deprecated functions, if you used them in your C++ code you need to use the new versions, otherwise the redirect is done for you in Blueprints automatically. The following functions were renamed in favour of other name:
	- `GetDialogueGuid` -> `GetDialogueGUID`
	- `GetActiveSpeakerState` -> `GetActiveNodeSpeakerState`
	- `GetActiveParticipantIcon` -> `GetActiveNodeParticipantIcon`
	- `GetActiveParticipant` -> `GetActiveNodeParticipant`
	- `GetActiveParticipantName` -> `GetActiveNodeParticipantName`
	- `GetDlgName` -> `GetDialogueName`
	- `GetDlgFName` -> `GetDialogueFName`
	- `GetDlgGuid` -> `GetDialogueGUID`
	- `RegisterDialogueModuleConsoleCommands` -> `RegisterDialogueConsoleCommands`
	- `UnRegisterDialogueModuleConsoleCommands` -> `UnregisterDialogueConsoleCommands`
	- `GetGenericData` -> `GetNodeGenericData`
	- `IsEdgeConnectedToVisitedNode` -> `IsOptionConnectedToVisitedNode`
	- `IsEdgeConnectedToEndNode` -> `IsOptionConnectedToEndNode`
	- `CouldStartDialogue` -> `CanStartDialogue`
	- `GetParticipantMap` -> `GetParticipantsMap`

# 9.0

## Important breaking change

- **Modified**  `IDlgDialogueParticipant` interface functions signature
	- Removed `const FName&` in favour of `FName` from all methods
	- Added `UDlgContext` as an argument to `CheckCondition` and `OnDialogueEvent`. This was done because if you start a Dialogue where the start (root) node has an enter event and in that enter event you want to get the current dialogue context you can't. Because the dialogue context returns after the start dialogue is called.

- **Renamed** `UDlgReflectionHelper` to `FNYReflectionHelper`

## Other Changes

- **Deprecated** `RegisterDialogueModuleConsoleCommands` and `UnRegisterDialogueModuleConsoleCommands`, you should use `RegisterDialogueConsoleCommands` and `UnregisterDialogueConsoleCommands` instead

- **Add** Custom Events and Conditions
	- Custom Event - Create a new blueprint with parent class `UDlgEventCustom` or `UDlgEventCustomHideCategories` (This is the same as UDlgEventCustom but it does NOT show the categories)
	- Custom Condition - Create a new blueprint with parent class `UDlgConditionCustom` or `UDlgConditionCustomHideCategories` ( This is the same as UDlgConditionCustom but it does NOT show the categories)

- **Add** setting option `bRegisterDialogueConsoleCommandsAutomatically` (default true) to automatically register the dialogue console commands on Begin Play

- **Improve** Error message on start dialogue when you give it a Blueprint Class instead of a Blueprint Instance

- **Improve** The Sound wave is now a `USoundBase` instead of a `USoundWave` to include more sound objects like cues.
  - **Added** `GetActiveNodeVoiceSoundBase` to the Dialogue Context. `GetActiveNodeVoiceSoundWave` just calls the SoundBase getter and casts to a `USoundWave`

- **Improvement** All getters of `UDlgNode` are blueprint accessible

- **Improvement** Reflection code is now engine version independent (so that it supports 4.25 more nicely)

- **Improvement** The Dialogue system gets the World from the game automatically (see `UDlgManager::GetDialogueWorld`), if you want to or need to set the  world manually, call `UDlgManager::SetPersistentWorldContextObject`

- **Fix** Null pointer check for LoadedWorld
- **Fix** Blueprint Nativization for 4.24

# 8.0.2

- **Fix** Linux Editor Compile

- **Add** `HasDialogueEnded()` function to the Dialogue Context

- **Add** `StartDialogueWithDefaultParticipants()` helper function to the Dialogue Manager
```cpp
	/**
	 * Starts a Dialogue with the provided Dialogue
	 * The function checks all the objects in the world to gather the participants
	 * This method can fail in the following situations:
	 *  - The Dialogue has a Participant which does not exist in the World
	 *	- Multiple Objects are using the same Participant Name in the World
	 *
	 * @returns The dialogue context object or nullptr if something went wrong
	 *
	*/
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Launch", meta = (WorldContext = "WorldContextObject"))
	static UDlgContext* StartDialogueWithDefaultParticipants(UObject* WorldContextObject, UDlgDialogue* Dialogue);
```

# 8.0.1

Fixed marketplace version of the plugin not handling renamed redirects properly.

# 8.0

## Important breaking change

Moved the config of the `DlgSystemSettings` from `DefaultEditorPerProjectUserSettings.ini` to the `DefaultEngine.ini`

**YOU NEED TO MOVE YOUR CONFIG MANUALLY AS TO AVOID LOSING YOUR CONFIG SETTINGS**

Reasons:
- The dialogue settings were already in the DlgSystem runtime module
- Previous location `DefaultEditorPerProjectUserSettings.ini` was only loaded for editor builds
- We have some runtime values in the dialogue settings

## Other changes

- **Deprecated** the *DlgText* text file format, the only supported format now is the JSON one, this will be removed in a future version.

- **Deprecated** the following functions: `GetActiveSpeakerState`, `GetActiveParticipantIcon`, `GetActiveParticipant`, `GetActiveParticipantName` in favour of variants with `Node` in their name. For now you will get a warning about this, but the deprecated functions will be removed in a future version.

- **Fix** localization support so that you can set the same namespace to multiple dialogues texts at the same time if you resave them.
**NOTE:** the text files do not export the localization information yet

- **Fix** Dialogue PostInitProperties spam (by checking for default class object and if not loaded)

- **Add** new logger type that also logs to the MessageLog. This is configurable in the dialogue settings

- **Add** more configuration options for the:
	- logger
	- localization/internationalization
	- default text edges

- **Add** option to the **File** menu to delete all dialogues text files

- **Disabled** by default exporting to any text file, as sometimes it does not work that well

- **Enabled** by default the SpeakerState visibility as it is a core feature

- **Disabled** by default the Voice Data fields visibility and recommend people use the **NodeData** instead

- **Add** option to search localization data inside the dialogue search

- **Add** function to check if a dialogue could be started or not based on the conditions of the first nodes

- **Renamed** some enums to not have any prefix, you can see the renames inside `Config/DefaultDlgSystem.ini`

## Known issues

- The text file exporter sometimes crashes on import/export, a fix will be implemented in the future.
- The text file exporter does not save localization data, so information is lost
