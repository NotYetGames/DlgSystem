// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgDialogue.h"

#include "UObject/DevObjectVersion.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#endif

#include "DlgSystemModule.h"
#include "IO/DlgConfigParser.h"
#include "IO/DlgConfigWriter.h"
#include "IO/DlgJsonWriter.h"
#include "IO/DlgJsonParser.h"
#include "Nodes/DlgNode_Speech.h"
#include "Nodes/DlgNode_End.h"
#include "DlgManager.h"
#include "Logging/DlgLogger.h"
#include "DlgHelper.h"

#define LOCTEXT_NAMESPACE "DlgDialogue"

// Unique DlgDialogue Object version id, generated with random
const FGuid FDlgDialogueObjectVersion::GUID(0x2B8E5105, 0x6F66348F, 0x2A8A0B25, 0x9047A071);
// Register Dialogue custom version with Core
FDevVersionRegistration GRegisterDlgDialogueObjectVersion(FDlgDialogueObjectVersion::GUID,
														  FDlgDialogueObjectVersion::LatestVersion, TEXT("Dev-DlgDialogue"));


// Update dialogue up to the ConvertedNodesToUObject version
void UpdateDialogueToVersion_ConvertedNodesToUObject(UDlgDialogue* Dialogue)
{
	// No Longer supported, get data from text file, and reconstruct everything
	Dialogue->InitialSyncWithTextFile();
#if WITH_EDITOR
	// Force clear the old graph
	Dialogue->ClearGraph();
#endif
}

// Update dialogue up to the UseOnlyOneOutputAndInputPin version
void UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(UDlgDialogue* Dialogue)
{
#if WITH_EDITOR
	Dialogue->GetDialogueEditorAccess()->UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(Dialogue);
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UObject interface
void UDlgDialogue::PreSave(const class ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);
	Name = GetDialogueFName();
	OnPreAssetSaved();
}

void UDlgDialogue::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FDlgDialogueObjectVersion::GUID);
	Super::Serialize(Ar);
	const int32 DialogueVersion = Ar.CustomVer(FDlgDialogueObjectVersion::GUID);
	if (DialogueVersion < FDlgDialogueObjectVersion::ConvertedNodesToUObject)
	{
		// No Longer supported
		return;
	}
}

void UDlgDialogue::PostLoad()
{
	Super::PostLoad();
	const int32 DialogueVersion = GetLinkerCustomVersion(FDlgDialogueObjectVersion::GUID);
	// Old files, UDlgNode used to be a FDlgNode
	if (DialogueVersion < FDlgDialogueObjectVersion::ConvertedNodesToUObject)
	{
		UpdateDialogueToVersion_ConvertedNodesToUObject(this);
	}

	// Simplified and reduced the number of pins (only one input/output pin), used for the new visualization
	if (DialogueVersion < FDlgDialogueObjectVersion::UseOnlyOneOutputAndInputPin)
	{
		UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(this);
	}

	// Simply the number of nodes, VirtualParent Node is merged into Speech Node and SelectRandom and SelectorFirst are merged into one Selector Node
	if (DialogueVersion < FDlgDialogueObjectVersion::MergeVirtualParentAndSelectorTypes)
	{
		FDlgLogger::Get().Warningf(
			TEXT("Dialogue = `%s` with Version MergeVirtualParentAndSelectorTypes will not be converted. See https://gitlab.com/snippets/1691704 for manual conversion"),
			*GetTextFilePathName()
		);
	}

	// Refresh the data, so that it is valid after loading.
	if (DialogueVersion < FDlgDialogueObjectVersion::AddTextFormatArguments ||
		DialogueVersion < FDlgDialogueObjectVersion::AddCustomObjectsToParticipantsData)
	{
		UpdateAndRefreshData();
	}

	// Create thew new GUID
	if (!HasGUID())
	{
		RegenerateGUID();
		FDlgLogger::Get().Debugf(
			TEXT("Creating new GUID = `%s` for Dialogue = `%s` because of of invalid GUID."),
			*GUID.ToString(), *GetPathName()
		);
	}

#if WITH_EDITOR
	const bool bHasDialogueEditorModule = GetDialogueEditorAccess().IsValid();
	// If this is false it means the graph nodes are not even created? Check for old files that were saved
	// before graph editor was even implemented. The editor will popup a prompt from FDialogueEditorUtilities::TryToCreateDefaultGraph
	if (bHasDialogueEditorModule && !GetDialogueEditorAccess()->AreDialogueNodesInSyncWithGraphNodes(this))
	{
		return;
	}
#endif

	// Check Nodes for validity
	const int32 NodesNum = Nodes.Num();
	for (int32 NodeIndex = 0; NodeIndex < NodesNum; NodeIndex++)
	{
		UDlgNode* Node = Nodes[NodeIndex];
#if WITH_EDITOR
		if (bHasDialogueEditorModule)
		{
			checkf(Node->GetGraphNode(), TEXT("Expected DialogueVersion = %d to have a valid GraphNode for Node index = %d :("), DialogueVersion, NodeIndex);
		}
#endif
		// Check children point to the right Node
		const TArray<FDlgEdge>& NodeEdges = Node->GetNodeChildren();
		const int32 EdgesNum = NodeEdges.Num();
		for (int32 EdgeIndex = 0; EdgeIndex < EdgesNum; EdgeIndex++)
		{
			const FDlgEdge& Edge = NodeEdges[EdgeIndex];
			if (!Edge.IsValid())
			{
				continue;
			}

			if (!Nodes.IsValidIndex(Edge.TargetIndex))
			{
				UE_LOG(
					LogDlgSystem,
					Fatal,
					TEXT("Node with index = %d does not have a valid Edge index = %d with TargetIndex = %d"),
					NodeIndex, EdgeIndex, Edge.TargetIndex
				);
			}
		}
	}

	bWasPostLoaded = true;
}

void UDlgDialogue::PostInitProperties()
{
	Super::PostInitProperties();

	// Ignore these cases
	if (HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		return;
	}

	const int32 DialogueVersion = GetLinkerCustomVersion(FDlgDialogueObjectVersion::GUID);

#if WITH_EDITOR
	// Wait for the editor module to be set by the editor in UDialogueGraph constructor
	if (GetDialogueEditorAccess().IsValid())
	{
		CreateGraph();
	}
#endif // #if WITH_EDITOR

	// Keep Name in sync with the file name
	Name = GetDialogueFName();

	// Used when creating new Dialogues
	// Initialize with a valid GUID
	if (DialogueVersion >= FDlgDialogueObjectVersion::AddGUID && !HasGUID())
	{
		RegenerateGUID();
		FDlgLogger::Get().Debugf(
			TEXT("Creating new GUID = `%s` for Dialogue = `%s` because of new created Dialogue."),
			*GUID.ToString(), *GetPathName()
		);
	}
}

void UDlgDialogue::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);
	Name = GetDialogueFName();
}

void UDlgDialogue::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	// Used when duplicating dialogues.
	// Make new guid for this copied Dialogue.
	RegenerateGUID();
	FDlgLogger::Get().Debugf(
		TEXT("Creating new GUID = `%s` for Dialogue = `%s` because Dialogue was copied."),
		*GUID.ToString(), *GetPathName()
	);
}

void UDlgDialogue::PostEditImport()
{
	Super::PostEditImport();

	// Used when duplicating dialogues.
	// Make new guid for this copied Dialogue
	RegenerateGUID();
	FDlgLogger::Get().Debugf(
		TEXT("Creating new GUID = `%s` for Dialogue = `%s` because Dialogue was copied."),
		*GUID.ToString(), *GetPathName()
	);
}

#if WITH_EDITOR
TSharedPtr<IDlgDialogueEditorAccess> UDlgDialogue::DialogueEditorAccess = nullptr;

bool UDlgDialogue::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	const bool bWasSaved = Super::Modify(bAlwaysMarkDirty);
	// if (StartNode)
	// {
	// 	bWasSaved = bWasSaved && StartNode->Modify(bAlwaysMarkDirty);
	// }

	// for (UDlgNode* Node : Nodes)
	// {
	// 	bWasSaved = bWasSaved && Node->Modify(bAlwaysMarkDirty);
	// }

	return bWasSaved;
}

void UDlgDialogue::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Signal to the listeners
	check(OnDialoguePropertyChanged.IsBound());
	OnDialoguePropertyChanged.Broadcast(PropertyChangedEvent);
}

void UDlgDialogue::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	UpdateAndRefreshData();

	const auto* ActiveMemberNode = PropertyChangedEvent.PropertyChain.GetActiveMemberNode();
	const auto* ActivePropertyNode = PropertyChangedEvent.PropertyChain.GetActiveNode();
	const FName MemberPropertyName = ActiveMemberNode && ActiveMemberNode->GetValue() ? ActiveMemberNode->GetValue()->GetFName() : NAME_None;
	const FName PropertyName = ActivePropertyNode && ActivePropertyNode->GetValue() ? ActivePropertyNode->GetValue()->GetFName() : NAME_None;

	// Check if the participant UClass implements our interface
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, ParticipantsClasses))
	{
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FDlgParticipantClass, ParticipantClass))
		{
			//const int32 ArrayIndex = PropertyChangedEvent.GetArrayIndex(MemberPropertyName.ToString());
			for (FDlgParticipantClass& Participant : ParticipantsClasses)
			{
				if (!IsValid(Participant.ParticipantClass))
				{
					continue;
				}

				if (!Participant.ParticipantClass->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
				{
					Participant.ParticipantClass = nullptr;
				}
			}
		}
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UDlgDialogue::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	// Add the graph to the list of referenced objects
	UDlgDialogue* This = CastChecked<UDlgDialogue>(InThis);
	Collector.AddReferencedObject(This->DlgGraph, This);
	Super::AddReferencedObjects(InThis, Collector);
}
// End UObject interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own functions
void UDlgDialogue::CreateGraph()
{
	// The Graph will only be null if this is the first time we are creating the graph for the Dialogue.
	// After the Dialogue asset is saved, the Dialogue will get the dialogue from the serialized uasset.
	if (DlgGraph != nullptr)
	{
		return;
	}

	if (!IsValid(StartNode))
	{
		StartNode = ConstructDialogueNode<UDlgNode_Speech>();
	}

	FDlgLogger::Get().Debugf(TEXT("Creating graph for Dialogue = `%s`"), *GetPathName());
	DlgGraph = GetDialogueEditorAccess()->CreateNewDialogueGraph(this);

	// Give the schema a chance to fill out any required nodes
	DlgGraph->GetSchema()->CreateDefaultNodesForGraph(*DlgGraph);
	MarkPackageDirty();
}

void UDlgDialogue::ClearGraph()
{
	if (!IsValid(DlgGraph))
	{
		return;
	}

	FDlgLogger::Get().Debugf(TEXT("Clearing graph for Dialogue = `%s`"), *GetPathName());
	GetDialogueEditorAccess()->RemoveAllGraphNodes(this);

	// Give the schema a chance to fill out any required nodes
	DlgGraph->GetSchema()->CreateDefaultNodesForGraph(*DlgGraph);
	MarkPackageDirty();
}

void UDlgDialogue::CompileDialogueNodesFromGraphNodes()
{
	if (!bCompileDialogue)
	{
		return;
	}

	FDlgLogger::Get().Infof(TEXT("Compiling Dialogue = `%s` (Graph data -> Dialogue data)`"), *GetPathName());
	GetDialogueEditorAccess()->CompileDialogueNodesFromGraphNodes(this);
}
#endif // #if WITH_EDITOR

void UDlgDialogue::ImportFromFile()
{
	// Simply ignore reloading
	const EDlgDialogueTextFormat TextFormat = GetDefault<UDlgSystemSettings>()->DialogueTextFormat;
	if (TextFormat == EDlgDialogueTextFormat::None)
	{
		UpdateAndRefreshData();
		return;
	}

	ImportFromFileFormat(TextFormat);
}

void UDlgDialogue::ImportFromFileFormat(EDlgDialogueTextFormat TextFormat)
{
	const bool bHasExtension = UDlgSystemSettings::HasTextFileExtension(TextFormat);
	const FString& TextFileName = GetTextFilePathName(TextFormat);

	// Nothing to do
	IFileManager& FileManager = IFileManager::Get();
	if (!bHasExtension)
	{
		// Useful For debugging
		if (TextFormat == EDlgDialogueTextFormat::All)
		{
			// Import from all
			const int32 TextFormatsNum = static_cast<int32>(EDlgDialogueTextFormat::NumTextFormats);
			for (int32 TextFormatIndex = static_cast<int32>(EDlgDialogueTextFormat::StartTextFormats);
					   TextFormatIndex < TextFormatsNum; TextFormatIndex++)
			{
				const EDlgDialogueTextFormat CurrentTextFormat = static_cast<EDlgDialogueTextFormat>(TextFormatIndex);
				const FString& CurrentTextFileName = GetTextFilePathName(CurrentTextFormat);
				if (FileManager.FileExists(*CurrentTextFileName))
				{
					ImportFromFileFormat(CurrentTextFormat);
				}
			}
		}
		return;
	}

	// File does not exist abort
	if (!FileManager.FileExists(*TextFileName))
	{
		FDlgLogger::Get().Errorf(TEXT("Reloading data for Dialogue = `%s` FROM file = `%s` FAILED, because the file does not exist"), *GetPathName(), *TextFileName);
		return;
	}

	// Clear data first
	StartNode = nullptr;
	Nodes.Empty();

	// TODO handle Name == NAME_None or invalid filename
	FDlgLogger::Get().Infof(TEXT("Reloading data for Dialogue = `%s` FROM file = `%s`"), *GetPathName(), *TextFileName);

	// TODO(vampy): Check for errors
	check(TextFormat != EDlgDialogueTextFormat::None);
	switch (TextFormat)
	{
		case EDlgDialogueTextFormat::JSON:
		{
			FDlgJsonParser JsonParser;
			JsonParser.InitializeParser(TextFileName);
			JsonParser.ReadAllProperty(GetClass(), this, this);
			break;
		}
		case EDlgDialogueTextFormat::DialogueDEPRECATED:
		{
			FDlgConfigParser Parser(TEXT("Dlg"));
			Parser.InitializeParser(TextFileName);
			Parser.ReadAllProperty(GetClass(), this, this);
			break;
		}
		default:
			checkNoEntry();
			break;

	}

	if (!IsValid(StartNode))
	{
		StartNode = ConstructDialogueNode<UDlgNode_Speech>();
	}

	// TODO(vampy): validate if data is legit, indicies exist and that sort.
	// Check if Guid is not a duplicate
	const TArray<UDlgDialogue*> DuplicateDialogues = UDlgManager::GetDialoguesWithDuplicateGUIDs();
	if (DuplicateDialogues.Num() > 0)
	{
		if (DuplicateDialogues.Contains(this))
		{
			// found duplicate of this Dialogue
			RegenerateGUID();
			FDlgLogger::Get().Warningf(
				TEXT("Creating new GUID = `%s` for Dialogue = `%s` because the input file contained a duplicate GUID."),
				*GUID.ToString(), *GetPathName()
			);
		}
		else
		{
			// We have bigger problems on our hands
			FDlgLogger::Get().Errorf(
				TEXT("Found Duplicate Dialogue that does not belong to this Dialogue = `%s`, DuplicateDialogues.Num = %d"),
				*GetPathName(),  DuplicateDialogues.Num()
			);
		}
	}

	Name = GetDialogueFName();
	AutoFixGraph();
	UpdateAndRefreshData(true);
}

void UDlgDialogue::OnPreAssetSaved()
{
#if WITH_EDITOR
	// Compile, graph data -> dialogue data
	CompileDialogueNodesFromGraphNodes();
#endif

	// Save file, dialogue data -> text file (.dlg)
	UpdateAndRefreshData(true);
	ExportToFile();
}

void UDlgDialogue::ExportToFile() const
{
	const EDlgDialogueTextFormat TextFormat = GetDefault<UDlgSystemSettings>()->DialogueTextFormat;
	if (TextFormat == EDlgDialogueTextFormat::None)
	{
		// Simply ignore saving
		return;
	}

	ExportToFileFormat(TextFormat);
}

void UDlgDialogue::ExportToFileFormat(EDlgDialogueTextFormat TextFormat) const
{
	// TODO(vampy): Check for errors
	const bool bHasExtension = UDlgSystemSettings::HasTextFileExtension(TextFormat);
	const FString& TextFileName = GetTextFilePathName(TextFormat);
	if (bHasExtension)
	{
		FDlgLogger::Get().Infof(TEXT("Exporting data for Dialogue = `%s` TO file = `%s`"), *GetPathName(), *TextFileName);
	}

	switch (TextFormat)
	{
		case EDlgDialogueTextFormat::JSON:
		{
			FDlgJsonWriter JsonWriter;
			JsonWriter.Write(GetClass(), this);
			JsonWriter.ExportToFile(TextFileName);
			break;
		}
		case EDlgDialogueTextFormat::DialogueDEPRECATED:
		{
			FDlgConfigWriter DlgWriter(TEXT("Dlg"));
			DlgWriter.Write(GetClass(), this);
			DlgWriter.ExportToFile(TextFileName);
			break;
		}
		case EDlgDialogueTextFormat::All:
		{
			// Useful for debugging
			// Export to all  formats
			const int32 TextFormatsNum = static_cast<int32>(EDlgDialogueTextFormat::NumTextFormats);
			for (int32 TextFormatIndex = static_cast<int32>(EDlgDialogueTextFormat::StartTextFormats);
					   TextFormatIndex < TextFormatsNum; TextFormatIndex++)
			{
				const EDlgDialogueTextFormat CurrentTextFormat = static_cast<EDlgDialogueTextFormat>(TextFormatIndex);
				ExportToFileFormat(CurrentTextFormat);
			}
			break;
		}
		default:
			// It Should not have any extension
			check(!bHasExtension);
			break;
	}
}

FDlgParticipantData& UDlgDialogue::GetParticipantDataEntry(FName ParticipantName, FName FallbackParticipantName, bool bCheckNone, const FString& ContextMessage)
{
	// Used to ignore some participants
	static FDlgParticipantData BlackHoleParticipant;

	// If the Participant Name is not set, it adopts the Node Owner Name
	const FName& ValidParticipantName = ParticipantName == NAME_None ? FallbackParticipantName : ParticipantName;

	// Parent/child is not valid, simply do nothing
	if (bCheckNone && ValidParticipantName == NAME_None)
	{
		FDlgLogger::Get().Warningf(
			TEXT("Ignoring ParticipantName = None, Context = `%s`. Either your node participant name is None or your participant name is None."),
			*ContextMessage
		);
		return BlackHoleParticipant;
	}

	return ParticipantsData.FindOrAdd(ValidParticipantName);
}

void UDlgDialogue::AddConditionsDataFromNodeEdges(const UDlgNode* Node, int32 NodeIndex)
{
	const FString NodeContext = FString::Printf(TEXT("Node %s"), NodeIndex > INDEX_NONE ? *FString::FromInt(NodeIndex) : TEXT("Start") );
	const FName FallbackParticipantName = Node->GetNodeParticipantName();

	for (const FDlgEdge& Edge : Node->GetNodeChildren())
	{
		const int32 TargetIndex = Edge.TargetIndex;

		for (const FDlgCondition& Condition : Edge.Conditions)
		{
			if (Condition.IsParticipantInvolved())
			{
				const FString ContextMessage = FString::Printf(TEXT("Adding Edge primary condition data from %s to Node %d"), *NodeContext, TargetIndex);
				GetParticipantDataEntry(Condition.ParticipantName, FallbackParticipantName, true, ContextMessage)
					.AddConditionPrimaryData(Condition);
			}
			if (Condition.IsSecondParticipantInvolved())
			{
				const FString ContextMessage = FString::Printf(TEXT("Adding Edge secondary condition data from %s to Node %d"), *NodeContext, TargetIndex);
				GetParticipantDataEntry(Condition.OtherParticipantName, FallbackParticipantName, true, ContextMessage)
					.AddConditionSecondaryData(Condition);
			}
		}
	}
}

void UDlgDialogue::RebuildAndUpdateNode(UDlgNode* Node, const UDlgSystemSettings& Settings, bool bUpdateTextsNamespacesAndKeys)
{
	static constexpr bool bEdges = true;
	static constexpr bool bUpdateGraphNode = false;

	// Rebuild & Update
	// NOTE: this can do a dialogue data -> graph node data update
	Node->RebuildTextArguments(bEdges, bUpdateGraphNode);
	Node->UpdateTextsValuesFromDefaultsAndRemappings(Settings, bEdges, bUpdateGraphNode);
	if (bUpdateTextsNamespacesAndKeys)
	{
		Node->UpdateTextsNamespacesAndKeys(Settings, bEdges, bUpdateGraphNode);
	}

	// Sync with the editor aka bUpdateGraphNode = true
	Node->UpdateGraphNode();
}

void UDlgDialogue::UpdateAndRefreshData(bool bUpdateTextsNamespacesAndKeys)
{
	FDlgLogger::Get().Infof(TEXT("Refreshing data for Dialogue = `%s`"), *GetPathName());

	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	ParticipantsData.Empty();
	AllSpeakerStates.Empty();

	// do not forget about the edges of the Root/Start Node
	if (IsValid(StartNode))
	{
		AddConditionsDataFromNodeEdges(StartNode, INDEX_NONE);
		RebuildAndUpdateNode(StartNode, *Settings, bUpdateTextsNamespacesAndKeys);
	}

	// Regular Nodes
	const int32 NodesNum = Nodes.Num();
	for (int32 NodeIndex = 0; NodeIndex < NodesNum; NodeIndex++)
	{
		const FString NodeContext = FString::Printf(TEXT("Node %d"), NodeIndex);
		UDlgNode* Node = Nodes[NodeIndex];
		const FName NodeParticipantName = Node->GetNodeParticipantName();

		// Rebuild & Update
		RebuildAndUpdateNode(Node, *Settings, bUpdateTextsNamespacesAndKeys);

		// participant names
		TArray<FName> Participants;
		Node->GetAssociatedParticipants(Participants);
		for (const FName& Participant : Participants)
		{
			if (!ParticipantsData.Contains(Participant))
			{
				ParticipantsData.Add(Participant);
			}
		}

		// gather SpeakerStates
		Node->AddAllSpeakerStatesIntoSet(AllSpeakerStates);

		// Conditions from nodes
		for (const FDlgCondition& Condition : Node->GetNodeEnterConditions())
		{
			if (Condition.IsParticipantInvolved())
			{
				const FString ContextMessage = FString::Printf(TEXT("Adding primary condition data for %s"), *NodeContext);
				GetParticipantDataEntry(Condition.ParticipantName, NodeParticipantName, true, ContextMessage)
					.AddConditionPrimaryData(Condition);
			}
			if (Condition.IsSecondParticipantInvolved())
			{
				const FString ContextMessage = FString::Printf(TEXT("Adding secondary condition data for %s"), *NodeContext);
				GetParticipantDataEntry(Condition.OtherParticipantName, NodeParticipantName, true, ContextMessage)
					.AddConditionSecondaryData(Condition);
			}
		}

		// Gather Edge Data
		AddConditionsDataFromNodeEdges(Node, NodeIndex);

		// Walk over edges of speaker nodes
		// NOTE: for speaker sequence nodes, the inner edges are handled by AddAllSpeakerStatesIntoSet
		// so no need to special case handle it
		const int32 NumNodeChildren = Node->GetNumNodeChildren();
		for (int32 EdgeIndex = 0; EdgeIndex < NumNodeChildren; EdgeIndex++)
		{
			const FDlgEdge& Edge = Node->GetNodeChildAt(EdgeIndex);
			const int32 TargetIndex = Edge.TargetIndex;

			// Speaker states
			AllSpeakerStates.Add(Edge.SpeakerState);

			// Text arguments are rebuild from the Node
			for (const FDlgTextArgument& TextArgument : Edge.GetTextArguments())
			{
				const FString ContextMessage = FString::Printf(TEXT("Adding Edge text arguments data from %s, to Node %d"), *NodeContext, TargetIndex);
				GetParticipantDataEntry(TextArgument.ParticipantName, NodeParticipantName, true, ContextMessage)
					.AddTextArgumentData(TextArgument);
			}
		}

		// Events
		for (const FDlgEvent& Event : Node->GetNodeEnterEvents())
		{
			const FString ContextMessage = FString::Printf(TEXT("Adding events data for %s"), *NodeContext);
			GetParticipantDataEntry(Event.ParticipantName, NodeParticipantName, true, ContextMessage)
				.AddEventData(Event);
		}

		// Text arguments
		for (const FDlgTextArgument& TextArgument : Node->GetTextArguments())
		{
			const FString ContextMessage = FString::Printf(TEXT("Adding text arguments data for %s"), *NodeContext);
			GetParticipantDataEntry(TextArgument.ParticipantName, NodeParticipantName, true, ContextMessage)
				.AddTextArgumentData(TextArgument);
		}
	}

	// Remove default values
	AllSpeakerStates.Remove(FName(NAME_None));

	//
	// Fill ParticipantClasses
	//
	TSet<FName> Participants;
	GetAllParticipantNames(Participants);

	// 1. remove outdated entries
	for (int32 Index = ParticipantsClasses.Num() - 1; Index >= 0; --Index)
	{
		const FName ExaminedName = ParticipantsClasses[Index].ParticipantName;
		if (!Participants.Contains(ExaminedName) || ExaminedName.IsNone())
		{
			ParticipantsClasses.RemoveAtSwap(Index);
		}

		Participants.Remove(ExaminedName);
	}

	// 2. add new entries
	for (const FName Participant : Participants)
	{
		if (Participant != NAME_None)
		{
			ParticipantsClasses.Add({ Participant, nullptr });
		}
		else
		{
			FDlgLogger::Get().Warning(TEXT("Trying to fill ParticipantsClasses, got a Participant name = None. Ignoring!"));
		}
	}

	// 3. Set auto default participant classes
	if (bWasPostLoaded && Settings->bAutoSetDefaultParticipantClasses)
	{
		TArray<UClass*> NativeClasses;
		TArray<UClass*> BlueprintClasses;
		FDlgHelper::GetAllClassesImplementingInterface(UDlgDialogueParticipant::StaticClass(), NativeClasses, BlueprintClasses);

		const TMap<FName, TArray<FDlgClassAndObject>> NativeClassesMap = FDlgHelper::ConvertDialogueParticipantsClassesIntoMap(NativeClasses);
		const TMap<FName, TArray<FDlgClassAndObject>> BlueprintClassesMap = FDlgHelper::ConvertDialogueParticipantsClassesIntoMap(BlueprintClasses);

		for (FDlgParticipantClass& Struct : ParticipantsClasses)
		{
			// Participant Name is not set or Class is set, ignore
			if (Struct.ParticipantName == NAME_None || Struct.ParticipantClass != nullptr)
			{
				continue;
			}

			// Blueprint
			if (BlueprintClassesMap.Contains(Struct.ParticipantName))
			{
				const TArray<FDlgClassAndObject>& Array = BlueprintClassesMap.FindChecked(Struct.ParticipantName);
				if (Array.Num() == 1)
				{
					Struct.ParticipantClass = Array[0].Class;
				}
			}

			// Native last resort
			if (Struct.ParticipantClass == nullptr && NativeClassesMap.Contains(Struct.ParticipantName))
			{
				const TArray<FDlgClassAndObject>& Array = NativeClassesMap.FindChecked(Struct.ParticipantName);
				if (Array.Num() == 1)
				{
					Struct.ParticipantClass = Array[0].Class;
				}
			}
		}
	}
}

FGuid UDlgDialogue::GetNodeGUIDForIndex(int32 NodeIndex) const
{
	if (IsValidNodeIndex(NodeIndex))
	{
		return Nodes[NodeIndex]->GetGUID();
	}

	// Invalid GUID
	return FGuid{};
}

int32 UDlgDialogue::GetNodeIndexForGUID(const FGuid& NodeGUID) const
{
	if (const int32* NodeIndexPtr = NodesGUIDToIndexMap.Find(NodeGUID))
	{
		return *NodeIndexPtr;
	}

	return INDEX_NONE;
}

void UDlgDialogue::SetStartNode(UDlgNode* InStartNode)
{
	if (!InStartNode)
	{
		return;
	}

	StartNode = InStartNode;
	// UpdateGUIDToIndexMap(StartNode, INDEX_NONE);
}

void UDlgDialogue::SetNodes(const TArray<UDlgNode*>& InNodes)
{
	Nodes = InNodes;
	for (int32 NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++)
	{
		UpdateGUIDToIndexMap(Nodes[NodeIndex], NodeIndex);
	}
}

void UDlgDialogue::SetNode(int32 NodeIndex, UDlgNode* InNode)
{
	if (!IsValidNodeIndex(NodeIndex) || !InNode)
	{
		return;
	}

	Nodes[NodeIndex] = InNode;
	UpdateGUIDToIndexMap(InNode, NodeIndex);
}

void UDlgDialogue::UpdateGUIDToIndexMap(const UDlgNode* Node, int32 NodeIndex)
{
	if (!Node || !IsValidNodeIndex(NodeIndex) || !Node->HasGUID())
	{
		return;
	}

	NodesGUIDToIndexMap.Add(Node->GetGUID(), NodeIndex);
}

bool UDlgDialogue::IsEndNode(int32 NodeIndex) const
{
	if (!Nodes.IsValidIndex(NodeIndex))
	{
		return false;
	}

	return Nodes[NodeIndex]->IsA<UDlgNode_End>();
}

void UDlgDialogue::AutoFixGraph()
{
	check(StartNode);
	// syntax correction 1: if there is no start node, we create one pointing to the first node
	if (StartNode->GetNodeChildren().Num() == 0 && Nodes.Num() > 0)
	{
		StartNode->AddNodeChild({ 0 });
	}
	StartNode->SetFlags(RF_Transactional);

	// syntax correction 2: if there is no end node, we add one
	bool bHasEndNode = false;
	// check if the end node is already there
	for (UDlgNode* Node : Nodes)
	{
		check(Node);
		Node->SetFlags(RF_Transactional);
		if (Node->IsA<UDlgNode_End>())
		{
			bHasEndNode = true;
			break;
		}
	}
	// add it if not
	if (!bHasEndNode && Nodes.Num() > 0)
	{
		auto* EndNode = ConstructDialogueNode<UDlgNode_End>();
		EndNode->SetNodeParticipantName(Nodes[0]->GetNodeParticipantName());
		Nodes.Add(EndNode);
	}

	// syntax correction 3: if a node is not an end node but has no children it will "adopt" the next node
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	for (int32 i = 0; i < Nodes.Num() - 1; ++i)
	{
		UDlgNode* Node = Nodes[i];
		const TArray<FDlgEdge>& NodeChildren = Node->GetNodeChildren();

		if (!Node->IsA<UDlgNode_End>() && NodeChildren.Num() == 0)
		{
			Node->AddNodeChild({ i + 1 });
		}

		// Add some text to the edges.
		Node->UpdateTextsValuesFromDefaultsAndRemappings(*Settings, true, true);
	}
}

FString UDlgDialogue::GetTextFilePathName(bool bAddExtension/* = true*/) const
{
	return GetTextFilePathName(GetDefault<UDlgSystemSettings>()->DialogueTextFormat, bAddExtension);
}

FString UDlgDialogue::GetTextFilePathName(EDlgDialogueTextFormat TextFormat, bool bAddExtension/* = true*/) const
{
		// Extract filename from path
	// NOTE: this is not a filesystem path, it is an unreal path 'Outermost.[Outer:]Name'
	// Usually GetPathName works, but the path name might be weird.
	// FSoftObjectPath(this).ToString(); which does call this function GetPathName() but it returns a legit clean path
	// if it is in the wrong format
	FString TextFileName = GetTextFilePathNameFromAssetPathName(FSoftObjectPath(this).ToString());
	if (bAddExtension)
	{
		// Modify the extension of the base text file depending on the extension
		TextFileName += UDlgSystemSettings::GetTextFileExtension(TextFormat);
	}

	return TextFileName;
}

bool UDlgDialogue::DeleteTextFileForTextFormat(EDlgDialogueTextFormat TextFormat) const
{
	return DeleteTextFileForExtension(UDlgSystemSettings::GetTextFileExtension(TextFormat));
}

bool UDlgDialogue::DeleteTextFileForExtension(const FString& FileExtension) const
{
	const FString TextFilePathName = GetTextFilePathName(false);
	if (TextFilePathName.IsEmpty())
	{
		// Memory corruption? tread carefully here
		FDlgLogger::Get().Errorf(
			TEXT("Can't delete text file for Dialogue = `%s` because the file path name is empty :O"),
			*GetPathName()
		);
		return false;
	}

	const FString FullPathName = TextFilePathName + FileExtension;
	return FDlgHelper::DeleteFile(FullPathName);
}

bool UDlgDialogue::DeleteAllTextFiles() const
{
	bool bStatus = true;
	for (const FString& FileExtension : GetDefault<UDlgSystemSettings>()->GetAllTextFileExtensions())
	{
		bStatus &= DeleteTextFileForExtension(FileExtension);
	}
	return bStatus;
}

bool UDlgDialogue::IsInProjectDirectory() const
{
	return FDlgHelper::IsPathInProjectDirectory(GetPathName());
}

FString UDlgDialogue::GetTextFilePathNameFromAssetPathName(const FString& AssetPathName)
{
	static const TCHAR* Separator = TEXT("/");

	// Get rid of the extension from `filename.extension` from the end of the path
	FString PathName = FPaths::GetBaseFilename(AssetPathName, false);

	// Get rid of the first folder, Game/ or Name/ (if in the plugins dir) from the beginning of the path.
	// Are we in the game directory?
	FString ContentDir = FPaths::ProjectContentDir();
	if (!PathName.RemoveFromStart(TEXT("/Game/")))
	{
		// We are in the plugins dir
		TArray<FString> PathParts;
		PathName.ParseIntoArray(PathParts, Separator);
		if (PathParts.Num() > 0)
		{
			const FString PluginName = PathParts[0];
			const FString PluginDir = FPaths::ProjectPluginsDir() / PluginName;

			// Plugin exists
			if (FPaths::DirectoryExists(PluginDir))
			{
				ContentDir = PluginDir / TEXT("Content/");
			}

			// remove plugin name
			PathParts.RemoveAt(0);
			PathName = FString::Join(PathParts, Separator);
		}
	}

	return ContentDir + PathName;
}


// End own functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
