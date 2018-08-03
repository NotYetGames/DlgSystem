// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "DlgHumanReadableTextCommandlet.h"

#include "Paths.h"
#include "PlatformFilemanager.h"
#include "GenericPlatformFile.h"
#include "DlgManager.h"
#include "Package.h"
#include "FileHelper.h"

#include "Nodes/DlgNode_Speech.h"
#include "DlgJsonWriter.h"
#include "DlgNode_SpeechSequence.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"


DEFINE_LOG_CATEGORY(LogDlgHumanReadableTextCommandlet);

UDlgHumanReadableTextCommandlet::UDlgHumanReadableTextCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}


int32 UDlgHumanReadableTextCommandlet::Main(const FString& Params)
{
	UE_LOG(LogDlgHumanReadableTextCommandlet, Display, TEXT("Starting"));

	// Parse command line - we're interested in the param vals
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> ParamVals;
	UCommandlet::ParseCommandLine(*Params, Tokens, Switches, ParamVals);

	bool bExport = false;
	bool bImport = false;

	// Set the output directory
	const FString* OutputInputDirectoryVal = ParamVals.Find(FString(TEXT("OutputInputDirectory")));
	if (OutputInputDirectoryVal == nullptr)
	{
		UE_LOG(LogDlgHumanReadableTextCommandlet, Error, TEXT("Did not provide argument -OutputInputDirectory=<Path>"));
		return -1;
	}
	OutputInputDirectory = *OutputInputDirectoryVal;

	if (OutputInputDirectory.IsEmpty())
	{
		UE_LOG(LogDlgHumanReadableTextCommandlet, Error, TEXT("OutputInputDirectory is empty, please provide a non empty one with -OutputInputDirectory=<Path>"));
		return -1;
	}

	// Make it absolute
	if (FPaths::IsRelative(OutputInputDirectory))
	{
		OutputInputDirectory = FPaths::Combine(FPaths::ProjectDir(), OutputInputDirectory);
	}

	if (Switches.Contains(TEXT("Export")))
	{
		bExport = true;
	}
	else if (Switches.Contains("Import"))
	{
		bImport = true;
	}
	if (!bExport && !bImport)
	{
		UE_LOG(LogDlgHumanReadableTextCommandlet, Error, TEXT("Did not choose any operationg. Either -export OR -import"));
		return -1;
	}

	// Create destination file
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*OutputInputDirectory) && PlatformFile.CreateDirectoryTree(*OutputInputDirectory))
	{
		UE_LOG(LogDlgHumanReadableTextCommandlet, Display, TEXT("Creating OutputInputDirectory = `%s`"), *OutputInputDirectory);
	}

	UDlgManager::LoadAllDialoguesIntoMemory();

	if (bExport)
		Export();
	else if (bImport)
		Import();

	return 0;
}

void UDlgHumanReadableTextCommandlet::Export()
{
	const TArray<UDlgDialogue*> AllDialogues = UDlgManager::GetAllDialoguesFromMemory();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	for (const UDlgDialogue* Dialogue : AllDialogues)
	{
		UPackage* Package = Dialogue->GetOutermost();
		check(Package);
		const FString OriginalDialoguePath = Package->GetPathName();
		FString DialoguePath = OriginalDialoguePath;

		// Only export game dialogues
		if (!DialoguePath.RemoveFromStart(TEXT("/Game")))
		{
			UE_LOG(LogDlgHumanReadableTextCommandlet, Warning, TEXT("Dialogue = `%s` is not in the game directory, ignoring"), *DialoguePath);
			continue;
		}

		const FString FileName = FPaths::GetBaseFilename(DialoguePath);
		const FString Directory = FPaths::GetPath(DialoguePath);

		// Ensure directory tree
		const FString FileSystemDirectoryPath = OutputInputDirectory / Directory;
		if (!PlatformFile.DirectoryExists(*FileSystemDirectoryPath) && PlatformFile.CreateDirectoryTree(*FileSystemDirectoryPath))
		{
			UE_LOG(LogDlgHumanReadableTextCommandlet, Display, TEXT("Creating directory = `%s`"), *FileSystemDirectoryPath);
		}

		// Export file
		FDlgJsonWriter JsonWriter;
		FDlgDialogue_FormatHumanReadable ExportFormat;
		ExportDialogueToHumanReadableFormat(*Dialogue, ExportFormat);
		JsonWriter.Write(FDlgDialogue_FormatHumanReadable::StaticStruct(), &ExportFormat);

		const FString FileSystemFilePath = FileSystemDirectoryPath / FileName + TEXT(".dlg_human.json");
		if (JsonWriter.ExportToFile(FileSystemFilePath))
		{
			UE_LOG(LogDlgHumanReadableTextCommandlet, Display, TEXT("Writing file = `%s` for Dialogue = `%s` "), *FileSystemFilePath, *OriginalDialoguePath);
		}
		else
		{
			UE_LOG(LogDlgHumanReadableTextCommandlet, Error, TEXT("FAILED to write file = `%s` for Dialogue = `%s`"), *FileSystemFilePath, *OriginalDialoguePath);
		}
	}
}

void UDlgHumanReadableTextCommandlet::Import()
{
	const TArray<UDlgDialogue*> AllDialogues = UDlgManager::GetAllDialoguesFromMemory();
	for (const UDlgDialogue* Dialogue : AllDialogues)
	{

	}
}

bool UDlgHumanReadableTextCommandlet::ExportDialogueToHumanReadableFormat(const UDlgDialogue& Dialogue, FDlgDialogue_FormatHumanReadable& OutFormat)
{
	OutFormat.DialogueName = Dialogue.GetDlgFName();
	OutFormat.DialogueGuid = Dialogue.GetDlgGuid();

	const TArray<UDlgNode*>& Nodes = Dialogue.GetNodes();
	for (int32 NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++)
	{
		const UDlgNode* Node = Nodes[NodeIndex];
		bool bFillEdges = true;
		if (const UDlgNode_Speech* NodeSpeech = Cast<UDlgNode_Speech>(Node))
		{
			if (NodeSpeech->IsVirtualParent() || NodeSpeech->GetRawNodeText().IsEmpty())
			{
				continue;
			}

			// Speech

			// Fill Nodes
			FDlgNodeSpeech_FormatHumanReadable ExportNode;
			ExportNodeToContext(Node, ExportNode.Context);
			ExportNode.NodeIndex = NodeIndex;
			ExportNode.Text = NodeSpeech->GetRawNodeText();
			OutFormat.SpeechNodes.Add(ExportNode);
		}
		else if (const UDlgNode_SpeechSequence* NodeSpeechSequence = Cast<UDlgNode_SpeechSequence>(Node))
		{
			// Speech Sequence

			FDlgNodeSpeechSequence_FormatHumanReadable ExportNode;
			ExportNodeToContext(Node, ExportNode.Context);
			ExportNode.NodeIndex = NodeIndex;

			for (const FDlgSpeechSequenceEntry& Entry : NodeSpeechSequence->GetNodeSpeechSequence())
			{
				FDlgSpeechSequenceEntry_FormatHumanReadable ExportEntry;
				ExportEntry.Speaker = Entry.Speaker;
				ExportEntry.EdgeText = Entry.EdgeText;
				ExportEntry.Text = Entry.Text;
				ExportNode.Sequence.Add(ExportEntry);
			}
			OutFormat.SpeechSequenceNodes.Add(ExportNode);
		}
		else
		{
			// not supported
		}

		// Fill Edges
		if (bFillEdges)
		{
			for (const FDlgEdge& Edge : Node->GetNodeChildren())
			{
				if (!Edge.IsValid() || Edge.Text.IsEmpty() || IsEdgeTextDefault(Edge.Text))
				{
					continue;
				}

				FDlgEdgeOrphan_FormatHumanReadable ExportEdge;
				ExportEdge.SourceNodeIndex = NodeIndex;
				ExportEdge.TargetNodeIndex = Edge.TargetIndex;
				ExportEdge.Text = Edge.Text;
				OutFormat.SpeechEdges.Add(ExportEdge);
			}
		}
	}

	return true;
}

bool UDlgHumanReadableTextCommandlet::ExportNodeToContext(const UDlgNode* Node, FDlgNodeContext_FormatHumanReadable& OutContext)
{
	if (Node == nullptr)
	{
		return false;
	}

	const UEdGraphNode* GraphNode = Node->GetGraphNode();
	if (GraphNode == nullptr)
	{
		return false;
	}

	const UDialogueGraphNode* DialogueGraphNode = Cast<UDialogueGraphNode>(GraphNode);
	if (DialogueGraphNode == nullptr)
	{
		return false;
	}

	for (const UDialogueGraphNode* ParentNode : DialogueGraphNode->GetParentNodes())
	{
		OutContext.ParentNodeIndices.Add(ParentNode->GetDialogueNodeIndex());
	}
	for (const UDialogueGraphNode* ChildNode : DialogueGraphNode->GetChildNodes())
	{
		OutContext.ChildNodeIndices.Add(ChildNode->GetDialogueNodeIndex());
	}
	OutContext.Speaker = Node->GetNodeParticipantName();

	return true;
}

bool UDlgHumanReadableTextCommandlet::IsEdgeTextDefault(const FText& EdgeText)
{
	return UDlgDialogue::EdgeTextFinish.EqualToCaseIgnored(EdgeText) || UDlgDialogue::EdgeTextNext.EqualToCaseIgnored(EdgeText);
}
