# v16

### Upgrade Notes
- The Private/Public directories don't exist anymore, everything is public now by default

### New Features

- Added String support for Dialogue Text Property Pick List suggestions.

### Bug fixes
- **Fixed** DialogueDataDisplay Icon.
- **Fixed** crash on editor launch when an AssetData has a null class.
- **Fixed** Crash exception inside [GCObjectReferencer.cpp:165](https://gitlab.com/NotYetGames/DlgSystem/-/merge_requests/13)

# v15
**Date:** March 8, 2022
### New Features

- **Added default class option** for `NodeData` which is used on `Speech` nodes and `SpeechSequence` node entries (Project Settings -> Editor -> Dialogue -> DefaultCustomNodeDataClass).
- **Added** `DlgParticipantName` **variable type** with custom picker which can be used to reference and access participants in custom events and conditions.
- **Added** different possible behaviors to handle non-end nodes without any satisfied children (Project Settings -> Editor -> Dialogue -> NoSatisfiedChildBehavior).
- **Added new node type:** `Proxy Node`. It jumps the execution to the target node without having a visible connection to it, can be used to reduce edge overlaps.
- `DlgContext` is modified to be more extandable, memory access is routed through it to support custom implementations (Credit goes to Marat Yakupov).
- **Added support for multiple** `Start` **nodes** to reduce visual complexity. `Start` nodes are evaluated from left to right until one has a satisfied child.
- The Random mode of the `Selector`node was exteneded to **support avoiding repetition and cycling through options.**
- `Speech` and `Selector` node colors are darker, more information is displayed on `Selector` nodes.
- Added custom node support, see `UDlgNode_Custom`
- **Added `EnterRestriction` parameter** on DlgNodes - can be used as a quicker way to make a node only enterable once
- Expose `IsNodeVisited` from the DlgContext inside Blueprint

### Deprecations

**DlgDialogue**
- `GetMutableStartNode` -> `GetMutableStartNodes`
- `GetStartNode` -> `GetStartNodes`
- `StartNode` -> `StartNodes`


# v14.1
**Date:** October 24, 2021
### Bug fixes
- **Fixed** Prevent helper blueprint nodes from being refreshed if the dialogues are not loaded into memory.

# v14
**Date:** August 28, 2021
### New Features
- Added the option to have an `UDlgNodeData` on the Edges, this works exactly like on the nodes.

### Deprecations

- **Dialogue Manager**
  - `ChooseOptionBasedOnAllOptionIndex` -> `ChooseOptionFromAll`
	- **NOTE**: This function used to check if the option was satisfied, it does not anymore. Use `IsOptionSatisfied` if you want to check for that.

### Bug fixes
- **Fixed** `GetOptionFromAll` because it was using the available children instead of all children.

# v13
**Date:** March 11, 2021
### Deprecations

Deprecated the following functions, because they were hard to use in BP in favour of variants that are easier to use.

This was done because using Arrays/Sets reference in BP can lead to some weird bugs.

- **Dialogue Manager**
  - `GetAllDialoguesParticipantNames` -> `GetDialoguesParticipantNames`
  - `GetAllDialoguesSpeakerStates` -> `GetDialoguesSpeakerStates`
  - `GetAllDialoguesIntNames` -> `GetDialoguesParticipantIntNames`
  - `GetAllDialoguesFloatNames` -> `GetDialoguesParticipantFloatNames`
  - `GetAllDialoguesBoolNames` -> `GetDialoguesParticipantBoolNames`
  - `GetAllDialoguesNameNames` -> `GetDialoguesParticipantFNameNames`
  - `GetAllDialoguesConditionNames` -> `GetDialoguesParticipantConditionNames`
  - `GetAllDialoguesEventNames` -> `GetDialoguesParticipantEventNames`
  - `GetAllObjectsWithDialogueParticipantInterface` -> `GetObjectsWithDialogueParticipantInterface`
  - `GetAllObjectsMapWithDialogueParticipantInterface` -> `GetObjectsMapWithDialogueParticipantInterface`
- **Dialogue Asset**
  - `GetAllParticipantNames` -> `GetParticipantNames`
  - `GetAllParticipantClasses` -> `GetParticipantClasses`
  - `GetAllSpeakerStates` -> `GetSpeakerStates`
  - `GetConditions` -> `GetParticipantConditionNames`
  - `GetEvents` -> `GetParticipantEventNames`
  - `GetCustomEvents` -> `GetParticipantCustomEvents`
  - `GetIntNames` -> `GetParticipantIntNames`
  - `GetBoolNames` -> `GetParticipantBoolNames`
  - `GetNameNames` -> `GetParticipantFNameNames`
  - `GetFloatNames` -> `GetParticipantFloatNames`
  - `GetClassIntNames` -> `GetParticipantClassIntNames`
  - `GetClassFloatNames` -> `GetParticipantClassFloatNames`
  - `GetClassBoolNames` -> `GetParticipantClassBoolNames`
  - `GetClassNameNames` -> `GetParticipantClassFNameNames`
  - `GetClassTextNames` -> `GetParticipantClassFTextNames`

### Add the following functions
- **Dialogue Asset**
  - `GetParticipantCustomConditions`
  - `GetParticipantCustomTextArguments`

### Blueprint

- Do not refresh pins if the Blueprint is not fully loaded
	- This stops refreshing the pins automatically on startup because it is not reliable (this happened all the time if the participant is inherited from another blueprint)
	- This change basically makes the refreshing of the pin names to be explicit (by the user) rather than implicit.
- Use wildcard pins for all Dialogue Select Nodes
### Bug fixes

- **Fixed** the `FDlgJsonWriter` so that it works in `Shipping` builds
- **Fixed** Linux compilation for UE `4.26`

# v12.3
**Date:** December 3, 2020

- **Transfer** `EnterConditions` and `EnterEvents` from first node when convert speech nodes to speech sequence
- **Added** support for **UE 4.26**

# v12.2
**Date:** November 30, 2020

- **Fixed** setting auto default participant classes on newly created Dialogues
- **Fixed** crash when running editor as dedicated server
- **Update** docs for the Interface class and update the example struct to have `SaveGame` enabled

# v12.1
**Date:** October 13, 2020

- **Moved** loading all dialogues logic into search manager init, but only after the content browser loads all the assets. This basically moves the loading to the editor so that it seems the plugin is not at fault for the long loading times.
- **Fixed** `CanStartDialogue` so that it matches the enter logic of `StartDialogue`

# V12
**Date:** September 30, 2020

- **Fix** crash when opening the Help Menu for UE 4.24 and 4.25

### Blueprint

- When **creating a new Blueprint** of type **Custom Events/Conditions/Text Argument** the overridden function/event is automatically created for you and the blueprint opens there.
- When **creating a new Blueprint** of type **Node Data** add a comment to the graph saying you need to add variables to see them in the Dialogue Editor, when using the Node Data

### Dialogue Editor
- **Add option** so that the **participant classes are auto set** to default to the first participant class that matches the participant Name
  - This only works if there is only one participant class of that Participant Name and Blueprint Classes take priority over Native Classes
- **Add details panel buttons** for "Browse to Asset on Content Browser" and "Open Asset" for the **Custom Events/Conditions/Text Argument or Node Data**
- **Nodes created** from another node will default to having the Participant Name the same as the Parent
- **Add** “Browse Participant in Content Browser” and “Open Participant Blueprint" for events and conditions when the participant is involved so that you can easier go the Participant.
  - This only works if the participant class is set in the particular Dialogue.

### Dialogue Content Browser
- **Add** support for Custom Events

### Find in Dialogue
- **Add the following search filters**:
  - Node GUIDs
  - Custom Object Names path (NOT the contents of the fields)
- **Add** the context Index for Events and Conditions so you know which array index it is from

### Content Browser
- **Allow to create** **Custom Events/Conditions/Text Argument or Node Data** from the Dialogue Menu (the same where you create the Dialogue Asset)
- **Move**  Dialogues filters to the content browser backend this way you can combine them with other filter
  - Only the Dialogue Participants Filter remains in the frontend because there is not sane way to implement it in the backend

- **Add** class and thumbnail icons for all custom classes (event, condition, text argument and node data)

### Dialogue
- Dialogue Participants Data now gathers data about all custom events/conditions/text arguments
- Increased Dialogue Version number to `AddCustomObjectsToParticipantsData`


# v11.1
**Date:** September 7, 2020

- **Important Bug Fixed** - Fix crash on compile (opened the dialogue asset or packaged the project) for some dialogues that have edges with the `WasNodeVisited` or `HasSatisfiedChild` Condition.

- **Set** namespace and key for default edge texts for "Finish" and "Next"
- **Add** the follow new methods to the **Dialogue Context**:
  - `IsValidOptionIndex`
  - `IsValidAllOptionIndex`
  - `GetActiveNodeParticipantDisplayName`

# v11
**Date:** August 29, 2020

## Important Changes

**Added GUIDs to Nodes**, this fixes [issue #30](https://gitlab.com/NotYetGames/DlgSystem/-/issues/30)

This is useful because node indices changes if you move nodes around, GUIDs are immutable, so it is
safer to use for game saves

Add the following new methods to the Dialogue:
- `IsValidNodeGUID`
- `GetGUIDForNodeIndex`
- `GetNodeIndexForGUID`
- `GetNode`
- `GetNodeFromGUID`

Add the follow new methods to the Dialogue Context:
- `GetActiveNodeGUID`
- `IsValidNodeGUID`
- `GetGUIDForNodeIndex`
- `GetNodeIndexForGUID`
- `GetVisitedNodeGUIDs`
- `GetNodeFromGUID`
- `WasNodeIndexVisitedInThisContext` (Renamed from `WasNodeVisitedInThisContext`)
- `WasNodeGUIDVisitedInThisContext`
- `StartFromNodeIndex`
- `StartFromNodeGUID`
- `StartFromNode`
- `ResumeDialogueFromNodeIndex` (Renamed from `ResumeDialogue`)
- `ResumeDialogueFromNodeGUID`


## Other Changes

- **Renamed** Dialogue Browser "Variables" to "Dialogue Values" so that is consistent with the rest of the plugin
- **Add** better tooltips documentation
- **Fix** Names for `IsObjectA<CustomStuff>` functions in the Blueprint Editor and **Add**  `IsObjectANodeData`
- **Fix** the **Virtual Parent Node** first satisfied child to fire its Enter Events if `bVirtualParentFireDirectChildEnterEvents = true` (by default)
- **Fix** the **Virtual Parent Node** to work with all other nodes types as a direct child (even tho this does not make sense in most cases the virtual parent nodes are usually just meant to be used for loops).
- **Improvement** - The direct child of the **Virtual Parent Node** is always marked as visited on the parent node OnEnter
- **Hide** the `bCheckChildrenOnEvaluation` on the **End Node** because it can't have any children

- **Removed** deprecated functions, if you used them in your C++ code you need to use the new versions, otherwise the redirect is done for you in Blueprints automatically. The following functions/properties were renamed in favour of other name:
	- `GetAllSpeakerState ` -> `GetAllSpeakerStates `
	- `SetPersistentWorldContextObject ` -> `SetDialoguePersistentWorldContextObject`
	- `ClearPersistentWorldContextObject ` -> `ClearDialoguePersistentWorldContextObject`
	- `GetParticipantNum ` -> `GetParticipantsNum`
	- `GetOptionNum ` -> `GetOptionsNum`
	- `GetAllOptionNum ` -> `GetAllOptionsNum`

- **Deprecated** some functions inside the **Dialogue Context** so that the names correspond to the rest
  of the functions:
  - `ChooseChild` in favour of `ChooseOption`
  - `ChooseChildBasedOnAllOptionIndex` in favour of `ChooseOptionBasedOnAllOptionIndex`
  - `ReevaluateChildren` in favour of `ReevaluateOptions`

- **Add** example blueprint and struct data for dialogue variable nodes inside the Plugin
- **Allow** convert speech nodes to speech sequence from right click on a speech node
- **Change** Participant Interface methods Category to be `Dialogue|Participant`
- **Multiplayer** changes:
  - Added the following methods to the `UDlgContext`: `ChooseSpeechSequenceOptionFromReplicated`, `GetActiveNodeAsSpeechSequence`
  - Added following methods to the `UDlgNode_SpeechSequence`: `OptionSelectedFromReplicated`, `GetSpeechSequenceIndex`

- **Add** `Jump to Node` button for Node Indices inside Conditions.
- **Renamed** Dialogue Browser Class Integers/Floats/etc to be just Integers/Floats/etc without the class prefix.

# v10
**Date:** July 15, 2020

- **Network replicate** the Dialogue Context (specifically the Dialogue and Participants)
- **Remove** `DlgText` format (the files with the `.dlg` file extension) from being selectable in the UI Dialogue Settings.
You can still set it manually in the config but this is not recommended as this format is DEPRECATED and is going to be removed
as an export method in the next version
- **Fixed** `UDlgContext::CanBeStarted` and `UDlgManager::CanStartDialogue` so that it does not always return false
  - `UDlgContext::CanBeStarted` is now a static function
- **Make** the Node **Children** Array to be view only (read only) and set it as settable option inside the settings (show by default)
- **Add** to whitelist `Linux Arm 64, Mac, Android, IOS, TVOS, HTML5, XboxOne, Switch, PS4, HoloLens, Lumin`. For most of these platforms we can't build ourselves but the runtime module should be platform agnostic anyways.
- **Add** The Dialogue Context as the first argument to `UDlgConditionCustom::IsConditionMet`. Only C++ users will have to add it manually in  their code.
- **Change** Plugin category to be `Misc`
- **Add** better tooltips to events/conditions/text arguments, it actually tells you what method it calls and where it calls it (dialogue interface or class of the participant).
- **Add** Custom Text Argument.
  - User Defined Text Argument, calls GetText on the custom text argument object.
    1. Create a new Blueprint derived from `DlgTextArgumentCustom` (or `DlgTextArgumentCustomHideCategories`)
    2. Override GetText
- **Move** All the Dialogue Filters into the same category in the Content Browser
- **Add** New Content Browse Filters for Custom Events, Conditions and Text Argument
- **Add** External Links to Not Yet (discord, plugins, forums, marketplace) inside the Help Menu and inside the Toolbar in the Dialogue Editor (can be hidden from the Dialogue Settings)
- **Move** The Dialogue Data Display to the proper Category under the Tools Menu

- **Removed** deprecated functions, if you used them in your C++ code you need to use the new versions, otherwise the redirect is done for you in Blueprints automatically. The following functions/properties were renamed in favour of other name:
	- `DlgSpeakerStates` -> `AllSpeakerStates`
	- `DlgData` -> `ParticipantsData`
	- `DlgParticipantClasses` -> `ParticipantsClasses`
	- `DlgVersion` -> `Version`
	- `DlgName` -> `Name`
	- `DlgGuid` -> `GUID`
	- `GetSpeakerStates` -> `GetAllSpeakerStates`
	- `GetParticipantData` -> `GetParticipantsData`
	- `IsParticipant` -> `HasParticipant`


# v9.1
**Date:** June 23, 2020

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

# v9
**Date:** May 3, 2020

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

# v8.0.2
**Date:** February 1, 2020

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

# v8.0.1

Fixed marketplace version of the plugin not handling renamed redirects properly.

# v8
**Date:** December 15, 2019

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
