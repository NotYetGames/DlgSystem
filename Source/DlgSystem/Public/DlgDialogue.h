// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Templates/SubclassOf.h"

#include "IDlgDialogueEditorAccess.h"
#include "DlgSystemSettings.h"
#include "DlgDialogueParticipantData.h"

#include "DlgDialogue.generated.h"

class UDlgNode;

// Custom serialization version for changes made in Dev-Dialogues stream
struct DLGSYSTEM_API FDlgDialogueObjectVersion
{
	enum Type
	{
		// Before any version changes were made
		BeforeCustomVersionWasAdded = 0,
		ConvertedNodesToUObject,
		UseOnlyOneOutputAndInputPin,
		MergeVirtualParentAndSelectorTypes,
		ConvertDialogueDataArraysToSets,
		AddGUID,
		AddComparisonWithOtherParticipant,
		AddTextFormatArguments,
		AddLocalizationOverwrittenNamespacesAndKeys,
		AddVirtualParentFireDirectChildEnterEvents,
		AddGUIDToNodes,
		AddCustomObjectsToParticipantsData,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	// The GUID for this custom version number
	const static FGuid GUID;

private:
	FDlgDialogueObjectVersion() {}
};


// Structure useful to cache all the names used by a participant
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgParticipantClass
{
	GENERATED_USTRUCT_BODY()

public:
	// The Name of the Participant Used inside this Dialogue
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue|Participant")
	FName ParticipantName = NAME_None;

	// The Participant Class corresponding for the ParticipantName
	// This is used to autocomplete and retrieve the Variables from that Class automatically when Using Class based Conditions/Events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Participant", meta = (MustImplement = "DlgDialogueParticipant"))
	UClass* ParticipantClass = nullptr;
};


/**
 *  Dialogue asset containing the static data of a dialogue
 *  Instances can be created in content browser
 *  Dialogues have a custom blueprint editor
 */
UCLASS(BlueprintType, Meta = (DisplayThumbnail = "true"))
class DLGSYSTEM_API UDlgDialogue : public UObject
{
	GENERATED_BODY()
public:

	//
	// Begin UObject Interface.
	//

	/** @return a one line description of an object for viewing in the thumbnail view of the generic browser */
	FString GetDesc() override { return TEXT(" DESCRIPTION = ") + GetName();  }

	/**
	 * Presave function. Gets called once before an object gets serialized for saving. This function is necessary
	 * for save time computation as Serialize gets called three times per object from within SavePackage.
	 *
	 * @warning: Objects created from within PreSave will NOT have PreSave called on them!!!
	 */
	void PreSave(const class ITargetPlatform* TargetPlatform) override;

	/** UObject serializer. */
	void Serialize(FArchive& Ar) override;

	/**
	 * Do any object-specific cleanup required immediately after loading an object,
	 * and immediately after any undo/redo.
	 */
	void PostLoad() override;

	/**
	 * Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	 * mainly this is to emulate some behavior of when the constructor was called after the properties were initialized.
	 * This creates the DlgGraph for this Dialogue.
	 */
	void PostInitProperties() override;

	/** Executed after Rename is executed. */
	void PostRename(UObject* OldOuter, FName OldName) override;

	/**
	 * Called after duplication & serialization and before PostLoad. Used to e.g. make sure UStaticMesh's UModel gets copied as well.
	 * Note: NOT called on components on actor duplication (alt-drag or copy-paste).  Use PostEditImport as well to cover that case.
	 */
	void PostDuplicate(bool bDuplicateForPIE) override;

	/**
	* Called after importing property values for this object (paste, duplicate or .t3d import)
	* Allow the object to perform any cleanup for properties which shouldn't be duplicated or
	* are unsupported by the script serialization
	*/
	void PostEditImport() override;

#if WITH_EDITOR
	/**
	 * Note that the object will be modified.  If we are currently recording into the
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 * @return true if the object was saved to the transaction buffer
	 */
	bool Modify(bool bAlwaysMarkDirty = true) override;

	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the FNYStructProperty member variable that contains the property that was modified.
	 */
	void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param InThis Object to collect references from.
	 * @param Collector	FReferenceCollector objects to be used to collect references.
	 */
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	// End UObject Interface.

	//
	// Begin own functions
	//

	// Broadcasts whenever a property of this dialogue changes.
	DECLARE_EVENT_OneParam(UDlgDialogue, FDialoguePropertyChanged, const FPropertyChangedEvent& /* PropertyChangedEvent */);
	FDialoguePropertyChanged OnDialoguePropertyChanged;

	// Helper functions to get the names of some properties. Used by the DlgSystemEditor module.
	static FName GetMemberNameName() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, Name); }
	static FName GetMemberNameGUID() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, GUID); }
	static FName GetMemberNameParticipantsData() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, ParticipantsData); }
	static FName GetMemberNameStartNode() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, StartNode); }
	static FName GetMemberNameNodes() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, Nodes); }

	// Create the basic dialogue graph.
	void CreateGraph();

	// Clears all nodes from the graph.
	void ClearGraph();

	// Gets the editor graph of this Dialogue.
	UEdGraph* GetGraph()
	{
		check(DlgGraph);
		return DlgGraph;
	}
	const UEdGraph* GetGraph() const
	{
		check(DlgGraph);
		return DlgGraph;
	}

	// Useful for initially compiling the Dialogue when we need the extra processing steps done by the compiler.
	void InitialCompileDialogueNodesFromGraphNodes()
	{
		if (bWasCompiledAtLeastOnce)
			return;

		CompileDialogueNodesFromGraphNodes();
		bWasCompiledAtLeastOnce = true;
	}

	// Compiles the dialogue nodes from the graph nodes. Meaning it transforms the graph data -> (into) dialogue data.
	void CompileDialogueNodesFromGraphNodes();

	// Sets the dialogue editor implementation. This is called in the constructor of the DlgDialogueGraph in the DlgSytemEditor module.
	static void SetDialogueEditorAccess(const TSharedPtr<IDlgDialogueEditorAccess>& InDialogueEditor)
	{
		check(!DialogueEditorAccess.IsValid());
		check(InDialogueEditor.IsValid());
		DialogueEditorAccess = InDialogueEditor;
	}

	// Gets the dialogue editor implementation.
	static TSharedPtr<IDlgDialogueEditorAccess> GetDialogueEditorAccess() { return DialogueEditorAccess; }

	// Enables/disables the compilation of the dialogues in the editor, use with care. Mainly used for optimization.
	void EnableCompileDialogue() { bCompileDialogue = true; }
	void DisableCompileDialogue() { bCompileDialogue = false; }
#endif

	// Construct and initialize a node within this Dialogue.
	template<class T>
	T* ConstructDialogueNode(TSubclassOf<UDlgNode> DialogueNodeClass = T::StaticClass())
	{
		// Set flag to be transactional so it registers with undo system
		T* DialogueNode = NewObject<T>(this, DialogueNodeClass, NAME_None, RF_Transactional);
		return DialogueNode;
	}

	//
	// Dialogue Data
	//

	// Gets the Dialogue Data Map. It maps Participant Name => Participant Data
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	const TMap<FName, FDlgParticipantData>& GetParticipantsData() const { return ParticipantsData; }

	// Checks if the provided ParticipantName (SpeakerName) is a key in the Dialogue Data Map
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool HasParticipant(FName ParticipantName) const { return ParticipantsData.Contains(ParticipantName); }

	// Gets the number of participants in the Dialogue Data Map.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetParticipantsNum() const { return ParticipantsData.Num(); }

	// Gets all the keys (participant names) of the DlgData Map
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetAllParticipantNames(TSet<FName>& OutSet) const
	{
		for (const auto& Element : ParticipantsData)
		{
			// Key is the ParticipantName
			OutSet.Add(Element.Key);
		}
	}

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	const TArray<FDlgParticipantClass>& GetAllParticipantClasses() const { return ParticipantsClasses; }

	/// EDITOR function, it only works if the participant class is setup in the ParticipantsClasses array
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UClass* GetParticipantClass(FName ParticipantName) const
	{
		for (const FDlgParticipantClass& Pair : ParticipantsClasses)
		{
			if (Pair.ParticipantName == ParticipantName)
			{
				return Pair.ParticipantClass;
			}
		}
		return nullptr;
	}


	// Gets the Condition Names that correspond to the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetConditions(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].Conditions);
		}
	}

	// Gets the Event Names that correspond to the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetEvents(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].Events);
		}
	}

	// Gets the Custom Events UClasses that correspond to the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetCustomEvents(FName ParticipantName, TSet<UClass*>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].CustomEvents);
		}
	}

	// Gets the int variable Names that correspond to the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetIntNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].IntVariableNames);
		}
	}

	// Gets the float variable Names that correspond to the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetFloatNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].FloatVariableNames);
		}
	}

	// Gets the bool variable Names that correspond to the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetBoolNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].BoolVariableNames);
		}
	}

	// Gets the name variable Names that correspond to the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetNameNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].NameVariableNames);
		}
	}


	// Gets the int variable Names that correspond to the UClass of the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetClassIntNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].ClassIntVariableNames);
		}
	}

	// Gets the float variable Names that correspond to the UClass of the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetClassFloatNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].ClassFloatVariableNames);
		}
	}

	// Gets the bool variable Names that correspond to the UClass of the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetClassBoolNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].ClassBoolVariableNames);
		}
	}

	// Gets the name variable Names that correspond to the UClass of the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetClassNameNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].ClassNameVariableNames);
		}
	}

	// Gets the FText variable Names that correspond to the UClass of the provided ParticipantName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetClassTextNames(FName ParticipantName, TSet<FName>& OutSet) const
	{
		if (ParticipantsData.Contains(ParticipantName))
		{
			OutSet.Append(ParticipantsData[ParticipantName].ClassTextVariableNames);
		}
	}

	// Gets all the SpeakerStates used inside this dialogue
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	void GetAllSpeakerStates(TSet<FName>& OutSet) const
	{
		OutSet.Append(AllSpeakerStates);
	}

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	int32 GetDialogueVersion() const { return Version; }

	// Gets/extracts the name (without extension) of the dialog from the uasset filename
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FString GetDialogueName() const
	{
		// Note: GetPathName() calls this at the end, so this just gets the direct name that we want.
		// Assumption only true for objects that have the Outer an UPackage.
		// Otherwise call FPaths::GetBaseFilename(GetPathName())
		return GetName();
	}

	// Same as the GetDialogueName only it returns a FName.
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FName GetDialogueFName() const { return GetFName(); }

	// Gets the unique identifier for this dialogue.
	UFUNCTION(BlueprintPure, Category = "Dialogue|GUID")
	FGuid GetGUID() const { check(GUID.IsValid()); return GUID; }

	// Regenerate the GUID of this Dialogue
	void RegenerateGUID() { GUID = FGuid::NewGuid(); }

	UFUNCTION(BlueprintPure, Category = "Dialogue|GUID")
	bool HasGUID() const { return GUID.IsValid(); }

	// Gets all the nodes
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	const TArray<UDlgNode*>& GetNodes() const { return Nodes; }

	// Gets the Start Node as a mutable pointer.
	UFUNCTION(BlueprintPure, Category = "Dialogue", DisplayName = "Get Start Node")
	UDlgNode* GetMutableStartNode() const { return StartNode; }
	const UDlgNode& GetStartNode() const { return *StartNode; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsValidNodeIndex(int32 NodeIndex) const { return Nodes.IsValidIndex(NodeIndex); }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsValidNodeGUID(const FGuid& NodeGUID) const { return IsValidNodeIndex(GetNodeIndexForGUID(NodeGUID)); }

	// Gets the GUID for the Node at NodeIndex
	UFUNCTION(BlueprintPure, Category = "Dialogue", DisplayName = "Get Node GUID For Index")
	FGuid GetNodeGUIDForIndex(int32 NodeIndex) const;

	// Gets the corresponding Node Index for the supplied NodeGUID
	// Returns -1 (INDEX_NONE) if the Node GUID does not exist.
	UFUNCTION(BlueprintPure, Category = "Dialogue", DisplayName = "Get Node Index For GUID")
	int32 GetNodeIndexForGUID(const FGuid& NodeGUID) const;

	// Gets the Node as a mutable pointer.
	UFUNCTION(BlueprintPure, Category = "Dialogue", DisplayName = "Get Node From Index")
	UDlgNode* GetMutableNodeFromIndex(int32 NodeIndex) const { return Nodes.IsValidIndex(NodeIndex) ? Nodes[NodeIndex] : nullptr; }

	UFUNCTION(BlueprintPure, Category = "Dialogue|Data", DisplayName = "Get Node From GUID")
    UDlgNode* GetMutableNodeFromGUID(const FGuid& NodeGUID) const { return GetMutableNodeFromIndex(GetNodeIndexForGUID(NodeGUID));   }

	// Sets a new Start Node. Use with care.
	void SetStartNode(UDlgNode* InStartNode);

	// NOTE: don't call this if you don't know what you are doing, you most likely need to call
	// SetStartNode
	// SetNodes
	// After this
	void EmptyNodesGUIDToIndexMap() { NodesGUIDToIndexMap.Empty(); }

	// Sets the Dialogue Nodes. Use with care.
	void SetNodes(const TArray<UDlgNode*>& InNodes);

	// Sets the Node at index NodeIndex. Use with care.
	void SetNode(int32 NodeIndex, UDlgNode* InNode);

	// Is the Node at NodeIndex (if it exists) an end node?
	bool IsEndNode(int32 NodeIndex) const;

	// Check if a text file in the same folder with the same name (Name) exists and loads the data from that file.
	void ImportFromFile();

	// Method to handle when this asset is going to be saved. Compiles the dialogue and saves to the text file.
	void OnPreAssetSaved();

	// Useful for initially reloading the data from the text file so that the dialogue is always in sync.
	void InitialSyncWithTextFile()
	{
		if (bIsSyncedWithTextFile)
		{
			return;
		}

		ImportFromFile();
		bIsSyncedWithTextFile = true;
	}

	// Exports this dialogue data into it's corresponding ".dlg" text file with the same name as this (Name).
	void ExportToFile() const;

	// Updates the data of some nodes
	// Fills the DlgData with the updated data
	// NOTE: this can do a dialogue data -> graph node data update
	void UpdateAndRefreshData(bool bUpdateTextsNamespacesAndKeys = false);

	// Adds a new node to this dialogue, returns the index location of the added node in the Nodes array.
	int32 AddNode(UDlgNode* NodeToAdd) { return Nodes.Add(NodeToAdd); }

	/**
	 * @param	bAddExtension	If this adds the .dlg or .dlg.json extension depending on the TextFormat.
	 * @return The path (as a relative path) and name of the text file, or empty string if something is wrong.
	 */
	FString GetTextFilePathName(bool bAddExtension = true) const;
	FString GetTextFilePathName(EDlgDialogueTextFormat TextFormat, bool bAddExtension = true) const;

	// Perform deletion on the text files
	bool DeleteTextFileForTextFormat(EDlgDialogueTextFormat TextFormat) const;
	bool DeleteTextFileForExtension(const FString& FileExtension) const;
	bool DeleteAllTextFiles() const;

	// Is this dialogue located inside the project directory
	bool IsInProjectDirectory() const;

	/**
	 * @return the text file path name (as a relative path) from the asset path name.
	 * NOTE: does not have extension, call GetTextFileExtension for that.
	 */
	static FString GetTextFilePathNameFromAssetPathName(const FString& AssetPathName);

private:
	// Adds conditions from the edges of this Node.
	void AddConditionsDataFromNodeEdges(const UDlgNode* Node, int32 NodeIndex);

	// Gets the map entry - creates it first if it is not yet there
	FDlgParticipantData& GetParticipantDataEntry(FName ParticipantName, FName FallbackParticipantName, bool bCheckNone, const FString& ContextMessage);

	// Rebuild & Update and node and its edges
	void RebuildAndUpdateNode(UDlgNode* Node, const UDlgSystemSettings& Settings, bool bUpdateTextsNamespacesAndKeys);

	void ImportFromFileFormat(EDlgDialogueTextFormat TextFormat);
	void ExportToFileFormat(EDlgDialogueTextFormat TextFormat) const;

	/**
	 * Tries to fix the internal graph of this Dialogue in the following ways:
	 * 1. If there is no start node, we create one pointing to the first node
	 * 2. If there is no end node, we add one
	 * 3. If a node is not an end node but has no children it will "adopt" the next node
	 */
	void AutoFixGraph();

	// Updates NodesGUIDToIndexMap with Node
	void UpdateGUIDToIndexMap(const UDlgNode* Node, int32 NodeIndex);

protected:
	// Used to keep track of the version in text  file too, besides being written in the .uasset file.
	UPROPERTY()
	int32 Version = FDlgDialogueObjectVersion::LatestVersion;

	// The name of the dialog, only used for reference in the text file, as this must always match the .uasset file name and .dlg file name
	UPROPERTY(VisibleAnywhere, Category = "Dialogue")
	FName Name;

	// The Unique identifier for each dialogue. This is used to uniquely identify a Dialogue, instead of it's name or path. Much more safer.
	UPROPERTY(VisibleAnywhere, Category = "Dialogue")
	FGuid GUID;

	// All the Participants that require for you to define its UClass otherwise the auto completion/suggestion won't work in case you want to modify/check Class variables.
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "Dialogue")
	TArray<FDlgParticipantClass> ParticipantsClasses;

	// Gathered data about events/conditions for each participant (for bp nodes, suggestions, etc.)
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Dialogue", Meta = (DlgNoExport))
	TMap<FName, FDlgParticipantData> ParticipantsData;

	// All the speaker states used inside this Dialogue.
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Dialogue", Meta = (DlgNoExport))
	TSet<FName> AllSpeakerStates;

	// Root node, Dialogue is started from the first child with satisfied condition (like the SelectorFirst node)
	// NOTE: Add VisibleAnywhere to make it easier to debug
	UPROPERTY(Instanced)
	UDlgNode* StartNode;

	// The new list of all nodes that belong to this Dialogue. Each nodes has children (edges) that have indices that point
	// to other nodes in this array.
	// NOTE: Add VisibleAnywhere to make it easier to debug
	UPROPERTY(AdvancedDisplay, EditFixedSize, Instanced, Meta = (DlgWriteIndex))
	TArray<UDlgNode*> Nodes;

	// Maps Node GUID => Node Index
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Dialogue", DisplayName = "Nodes GUID To Index Map")
	TMap<FGuid, int32> NodesGUIDToIndexMap;

	// Useful for syncing on the first run with the text file.
	bool bIsSyncedWithTextFile = false;

#if WITH_EDITORONLY_DATA
	// EdGraph based representation of the DlgDialogue class
	UPROPERTY(Meta = (DlgNoExport))
	UEdGraph* DlgGraph;

	// Ptr to interface to dialogue editor operations. See function SetDialogueEditorAccess for more details.
	static TSharedPtr<IDlgDialogueEditorAccess> DialogueEditorAccess;

	// Flag used for optimization, used to enable/disable compiling of the dialogue for bulk operations.
	bool bCompileDialogue = true;

	// Flag indicating if this Dialogue was compiled at least once in the current runtime.
	bool bWasCompiledAtLeastOnce = false;

	// Used to build the change event and broadcast it to the children
	int32 BroadcastPropertyNodeIndexChanged = INDEX_NONE;
#endif


	// Flag that indicates that PostLoad was called
	bool bWasPostLoaded = false;
};
