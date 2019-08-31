// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Templates/SubclassOf.h"

#include "DlgIDialogueEditorModule.h"
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
		AddGuid,
		AddComparisonWithOtherParticipant,
		AddTextFormatArguments,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	// The GUID for this custom version number
	const static FGuid GUID;

private:
	FDlgDialogueObjectVersion() {}
};


/** Structure useful to cache all the names used by a participant */
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgParticipantClass
{
	GENERATED_USTRUCT_BODY()

	/** FName based conditions (aka conditions of type DlgConditionEventCall). */
	UPROPERTY(VisibleAnywhere, Category = DialogueParticipantData)
	FName ParticipantName;

	/** FName based events (aka events of type EDlgEventType) */
	UPROPERTY(EditAnywhere, Category = DialogueParticipantData, meta = (MustImplement = "DlgDialogueParticipant"))
	UClass* ParticipantClass;
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

	// Begin UObject Interface.

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
	void PostRename(UObject* OldOuter, const FName OldName) override;

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
	 * Called by the editor to query whether a property of this object is allowed to be modified.
	 * The property editor uses this to disable controls for properties that should not be changed.
	 * When overriding this function you should always call the parent implementation first.
	 *
	 * @param	InProperty	The property to query
	 *
	 * @return	true if the property can be modified in the editor, otherwise false
	 */
	bool CanEditChange(const UProperty* InProperty) const override;

	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the UStructProperty member variable that contains the property that was modified.
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

	// Begin own functions
	/** Broadcasts whenever a property of this dialogue changes. */
	DECLARE_EVENT_OneParam(UDlgDialogue, FDialoguePropertyChanged, const FPropertyChangedEvent& /* PropertyChangedEvent */);
	FDialoguePropertyChanged OnDialoguePropertyChanged;

	/** Helper functions to get the names of some properties. Used by the DlgSystemEditor module. */
	static FName GetMemberNameDlgName() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, DlgName); }
	static FName GetMemberNameDlgGuid() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, DlgGuid); }
	static FName GetMemberNameDlgData() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, DlgData); }
	static FName GetMemberNameStartNode() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, StartNode); }
	static FName GetMemberNameNodes() { return GET_MEMBER_NAME_CHECKED(UDlgDialogue, Nodes); }

	/** Create the basic dialogue graph. */
	void CreateGraph();

	/** Clears all nodes from the graph. */
	void ClearGraph();

	/** Gets the editor graph of this Dialogue. */
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

	/** Useful for initially compiling the Dialogue when we need the extra processing steps done by the compiler. */
	void InitialCompileDialogueNodesFromGraphNodes()
	{
		if (bWasCompiledAtLeastOnce)
			return;

		CompileDialogueNodesFromGraphNodes();
		bWasCompiledAtLeastOnce = true;
	}

	/** Compiles the dialogue nodes from the graph nodes. Meaning it transofrms the graph data -> (into) dialogue data. */
	void CompileDialogueNodesFromGraphNodes();

	/** Sets the dialogue editor implementation. This is called in the constructor of the DlgDialogueGraph in the DlgSytemEditor module. */
	static void SetDialogueEditorModule(const TSharedPtr<IDlgDialogueEditorModule>& InDialogueEditor)
	{
		check(!DialogueEditorModule.IsValid());
		check(InDialogueEditor.IsValid());
		DialogueEditorModule = InDialogueEditor;
	}

	/** Gets the dialogue editor implementation. */
	static TSharedPtr<IDlgDialogueEditorModule> GetDialogueEditorModule() { return DialogueEditorModule; }

	// Enables/disables the compilation of the dialogues in the editor, use with care. Mainly used for optimization.
	void EnableCompileDialogue() { bCompileDialogue = true; }
	void DisableCompileDialogue() { bCompileDialogue = false; }
#endif

	/** Construct and initialize a node within this Dialogue. */
	template<class T>
	T* ConstructDialogueNode(TSubclassOf<UDlgNode> DialogueNodeClass = T::StaticClass())
	{
		// Set flag to be transactional so it registers with undo system
		T* DialogueNode = NewObject<T>(this, DialogueNodeClass, NAME_None, RF_Transactional);
		return DialogueNode;
	}

	// DlgData:
	/** Gets the Dialogue Data Map. It maps Participant Name => Participant Data */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	const TMap<FName, FDlgParticipantData>& GetParticipantData() const { return DlgData; }

	/** Checks if the provided ParticipantName (SpeakerName) is a key in the Dialogue Data Map */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	bool IsParticipant(const FName& ParticipantName) const { return DlgData.Contains(ParticipantName); }

	/** Gets the number of participants in the Dialogue Data Map. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	int32 GetParticipantNum() const { return DlgData.Num(); }

	/** Gets all the keys (participant names) of the DlgData Map */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetAllParticipantNames(TSet<FName>& OutSet) const
	{
		for (const auto& Element : DlgData)
		{
			// Key is the ParticipantName
			OutSet.Add(Element.Key);
		}
	}

	/** EDITOR function, it only works if the participant class is setup in the DlgParticipantClasses array */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	UClass* GetParticipantClass(const FName ParticipantName) const
	{
		for (const FDlgParticipantClass& Pair : DlgParticipantClasses)
		{
			if (Pair.ParticipantName == ParticipantName)
			{
				return Pair.ParticipantClass;
			}
		}
		return nullptr;
	}


	/** Gets the Condition Names that correspond to the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetConditions(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].Conditions);
		}
	}

	/** Gets the Event Names that correspond to the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetEvents(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].Events);
		}
	}

	/** Gets the int variable Names that correspond to the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetIntNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].IntVariableNames);
		}
	}

	/** Gets the float variable Names that correspond to the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetFloatNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].FloatVariableNames);
		}
	}

	/** Gets the bool variable Names that correspond to the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetBoolNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].BoolVariableNames);
		}
	}

	/** Gets the name variable Names that correspond to the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetNameNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].NameVariableNames);
		}
	}


	/** Gets the int variable Names that correspond to the UClass of the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetClassIntNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].ClassIntVariableNames);
		}
	}

	/** Gets the float variable Names that correspond to the UClass of the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetClassFloatNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].ClassFloatVariableNames);
		}
	}

	/** Gets the bool variable Names that correspond to the UClass of the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetClassBoolNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].ClassBoolVariableNames);
		}
	}

	/** Gets the name variable Names that correspond to the UClass of the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetClassNameNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].ClassNameVariableNames);
		}
	}

	/** Gets the FText variable Names that correspond to the UClass of the provided ParticipantName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetClassTextNames(const FName& ParticipantName, TSet<FName>& OutSet) const
	{
		if (DlgData.Contains(ParticipantName))
		{
			OutSet.Append(DlgData[ParticipantName].ClassTextVariableNames);
		}
	}

	/** Gets all the SpeakerStates used inside this dialogue */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	void GetAllSpeakerState(TSet<FName>& OutSet) const
	{
		OutSet.Append(DlgSpeakerStates);
	}

	UFUNCTION(BlueprintPure, Category = DialogueData)
	const TSet<FName>& GetSpeakerStates() const { return DlgSpeakerStates; }

	/** Gets/extracts the name (without extension) of the dialog from the uasset filename */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	FString GetDlgName() const
	{
		// Note: GetPathName() calls this at the end, so this just gets the direct name that we want.
		// Assumption only true for objects that have the Outer an UPackage.
		// Otherwise call FPaths::GetBaseFilename(GetPathName())
		return GetName();
	}

	/** Same as the GetDlgName only it returns a FName. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	FName GetDlgFName() const { return GetFName(); }

	/** Gets the unique identifier for this dialogue. */
	UFUNCTION(BlueprintPure, Category = DialogueData)
	FGuid GetDlgGuid() const { check(DlgGuid.IsValid()); return DlgGuid; }

	/** Regenerate the DlgGuid of this Dialogue */
	void RegenerateGuid() { DlgGuid = FGuid::NewGuid(); }

	/** Gets all the nodes */
	const TArray<UDlgNode*>& GetNodes() const { return Nodes; }

	/** Gets the Start Node */
	const UDlgNode& GetStartNode() const { return *StartNode; }

	/** Gets the Start Node as a mutable pointer. */
	UDlgNode* GetMutableStartNode() { return StartNode; }

	/** Gets the Node as a mutable pointer. */
	UDlgNode* GetMutableNode(int32 NodeIndex) { return Nodes.IsValidIndex(NodeIndex) ? Nodes[NodeIndex] : nullptr; }

	/** Sets a new Start Node. Use with care. */
	void SetStartNode(UDlgNode* InStartNode) { StartNode = InStartNode; }

	/** Sets the Dialogue Nodes. Use with care. */
	void SetNodes(const TArray<UDlgNode*>& InNodes) { Nodes = InNodes; }

	/** Sets the Node at index NodeIndex. Use with care. */
	void SetNode(int32 NodeIndex, UDlgNode* InNode) { Nodes[NodeIndex] = InNode; }

	/** Check if a ".dlg" text file in the same folder with the same name (DlgName) exists and loads the data from that file. */
	void ReloadFromFile();

	/** Method to handle when this asset is going to be saved. Compiles the dialogue and saves to the text file. */
	void OnAssetSaved();

	/** Useful for initially reloading the data from the text file so that the dialogue is always in sync. */
	void InitialSyncWithTextFile()
	{
		if (bIsSyncedWithTextFile)
		{
			return;
		}

		ReloadFromFile();
		bIsSyncedWithTextFile = true;
	}

	/** Exports this dialogue data into it's corresponding ".dlg" text file with the same name as this (DlgName). */
	void ExportToFile() const;

	/** Gathers data from the Dialogue to fill DlgData (used at runtime) */
	void RefreshData();

	/** Adds a new node to this dialogue, returns the index location of the added node in the Nodes array. */
	int32 AddNode(UDlgNode* NodeToAdd) { return Nodes.Add(NodeToAdd); }

	/**
	 * @param	bAddExtension	If this adds the .dlg or .dlg.json extension depending on the TextFormat.
	 * @return The path (as a relative path) and name of the text file, or empty string if something is wrong.
	 */
	FString GetTextFilePathName(bool bAddExtension = true) const;

	/**
	 * @return the text file path name (as a relative path) from the asset path name.
	 * NOTE: does not have extension, call GetTextFileExtension for that.
	 */
	static FString GetTextFilePathNameFromAssetPathName(const FString& AssetPathName);

	/** @return the extension of the text file depending on the InTextFormat. */
	static FString GetTextFileExtension(EDlgDialogueTextFormat InTextFormat);

private:
	/**
	 * Tries to fix the internal graph of this Dialogue in the following ways:
	 * 1. If there is no start node, we create one pointing to the first node
	 * 2. If there is no end node, we add one
	 * 3. If a node is not an end node but has no children it will "adopt" the next node
	 */
	void AutoFixGraph();

private:
	/** Used to keep track of the version in text  file too, besides being written in the .uasset file. */
	UPROPERTY()
	int32 DlgVersion = FDlgDialogueObjectVersion::LatestVersion;

	/** The name of the dialog, only used for reference in the text file, as this must always match the .uasset file name and .dlg file name */
	UPROPERTY(VisibleAnywhere, Category = DialogueData)
	FName DlgName;

	/** The Unique identifier for each dialogue. This is used to uniquely identify a Dialogue, instead of it's name or path. Much more safer. */
	UPROPERTY(VisibleAnywhere, Category = DialogueData)
	FGuid DlgGuid;

	/** All the Participants that require for you to define its UClass otherwise the auto completion/suggestion won't work in case you want to modify/check Class variables.  */
	UPROPERTY(EditAnywhere, EditFixedSize, Category = DialogueData)
	TArray<FDlgParticipantClass> DlgParticipantClasses;

	/** Gathered data about events/conditions for each participant (for bp nodes, suggestions, etc.) */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = DialogueData, Meta = (DlgNoExport))
	TMap<FName, FDlgParticipantData> DlgData;

	/** All the speaker states used inside this Dialogue. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = DialogueData, Meta = (DlgNoExport))
	TSet<FName> DlgSpeakerStates;

	/**
	 * Root node, Dialogue is started from the first child with satisfied condition (like the SelectorFirst node)
	 * NOTE: Add VisibleAnywhere to make it easier to debug
	 */
	UPROPERTY(Instanced)
	UDlgNode* StartNode;

	/**
	 * The new list of all nodes that belong to this Dialogue. Each nodes has children (edges) that have indices that point
	 * to other nodes in this array.
	 * NOTE: Add VisibleAnywhere to make it easier to debug
	 */
	UPROPERTY(AdvancedDisplay, EditFixedSize, Instanced, Meta = (DlgWriteIndex))
	TArray<UDlgNode*> Nodes;

	// Useful for syncing on the first run with the text file.
	bool bIsSyncedWithTextFile = false;

#if WITH_EDITORONLY_DATA
	// EdGraph based representation of the DlgDialogue class
	UPROPERTY(Meta = (DlgNoExport))
	UEdGraph* DlgGraph;

	// Ptr to interface to dialogue editor operations. See function SetDialogueEditorModule for more details.
	static TSharedPtr<IDlgDialogueEditorModule> DialogueEditorModule;

	/** Flag used for optimization, used to enable/disable compiling of the dialogue for bulk operations. */
	bool bCompileDialogue = true;

	/** Flag indicating if this Dialogue was compiled at least once in the current runtime. */
	bool bWasCompiledAtLeastOnce = false;

	// Used to build the change event and broadcast it to the children
	int32 BroadcastPropertyNodeIndexChanged = INDEX_NONE;
#endif

};
