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
