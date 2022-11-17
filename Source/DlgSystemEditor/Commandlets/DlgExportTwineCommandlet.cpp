// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.

#include "DlgExportTwineCommandlet.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "DlgSystem/NYEngineVersionHelpers.h"

#if NY_ENGINE_VERSION >= 500
    #include "HAL/PlatformFileManager.h"
#else
    #include "HAL/PlatformFilemanager.h"
#endif

#include "GenericPlatform/GenericPlatformFile.h"
#include "UObject/Package.h"
#include "FileHelpers.h"

#include "DlgSystem/DlgManager.h"
#include "DlgSystem/Nodes/DlgNode_Speech.h"
#include "DlgSystem/Nodes/DlgNode_SpeechSequence.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystem/DlgHelper.h"


DEFINE_LOG_CATEGORY(LogDlgExportTwineCommandlet);


const FString UDlgExportTwineCommandlet::TagNodeStart(TEXT("node-start"));
const FString UDlgExportTwineCommandlet::TagNodeEnd(TEXT("node-end"));
const FString UDlgExportTwineCommandlet::TagNodeVirtualParent(TEXT("node-virtual-parent"));
const FString UDlgExportTwineCommandlet::TagNodeSpeech(TEXT("node-speech"));
const FString UDlgExportTwineCommandlet::TagNodeSpeechSequence(TEXT("node-speech-sequence"));
const FString UDlgExportTwineCommandlet::TagNodeSelectorFirst(TEXT("node-selector-first"));
const FString UDlgExportTwineCommandlet::TagNodeSelectorRandom(TEXT("node-selector-random"));

const FIntPoint UDlgExportTwineCommandlet::SizeSmall(100, 100);
const FIntPoint UDlgExportTwineCommandlet::SizeWide(200, 100);
const FIntPoint UDlgExportTwineCommandlet::SizeTall(100, 200);
const FIntPoint UDlgExportTwineCommandlet::SizeLarge(200, 200);

TMap<FString, FString> UDlgExportTwineCommandlet::TwineTagNodesColorsMap;

UDlgExportTwineCommandlet::UDlgExportTwineCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}


int32 UDlgExportTwineCommandlet::Main(const FString& Params)
{
	UE_LOG(LogDlgExportTwineCommandlet, Display, TEXT("Starting"));

	InitTwinetagNodesColors();

	// Parse command line - we're interested in the param vals
	TArray<FString> Tokens;
	TArray<FString> Switches;
	TMap<FString, FString> ParamVals;
	UCommandlet::ParseCommandLine(*Params, Tokens, Switches, ParamVals);

	if (Switches.Contains(TEXT("Flatten")))
	{
		bFlatten = true;
	}

	// Set the output directory
	const FString* OutputDirectoryVal = ParamVals.Find(FString(TEXT("OutputDirectory")));
	if (OutputDirectoryVal == nullptr)
	{
		UE_LOG(LogDlgExportTwineCommandlet, Error, TEXT("Did not provide argument -OutputDirectory=<Path>"));
		return -1;
	}
	OutputDirectory = *OutputDirectoryVal;

	if (OutputDirectory.IsEmpty())
	{
		UE_LOG(LogDlgExportTwineCommandlet, Error, TEXT("OutputDirectory is empty, please provide a non empty one with -OutputDirectory=<Path>"));
		return -1;
	}

	// Make it absolute
	if (FPaths::IsRelative(OutputDirectory))
	{
		OutputDirectory = FPaths::Combine(FPaths::ProjectDir(), OutputDirectory);
	}

	// Create destination directory
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*OutputDirectory) && PlatformFile.CreateDirectoryTree(*OutputDirectory))
	{
		UE_LOG(LogDlgExportTwineCommandlet, Display, TEXT("Creating OutputDirectory = `%s`"), *OutputDirectory);
	}

	UDlgManager::LoadAllDialoguesIntoMemory();
	UE_LOG(LogDlgExportTwineCommandlet, Display, TEXT("Exporting to = `%s`"), *OutputDirectory);

	// Some Dialogues may be unclean?
	//FDlgCommandletHelper::SaveAllDialogues();

	// Keep track of all created files so that we don't have duplicates
	TSet<FString> CreateFiles;

	// Export to twine
	const TArray<UDlgDialogue*> AllDialogues = UDlgManager::GetAllDialoguesFromMemory();
	for (const UDlgDialogue* Dialogue : AllDialogues)
	{
		UPackage* Package = Dialogue->GetOutermost();
		check(Package);
		const FString OriginalDialoguePath = Package->GetPathName();
		FString DialoguePath = OriginalDialoguePath;

		// Only export game dialogues
		if (!FDlgHelper::IsPathInProjectDirectory(DialoguePath))
		{
			UE_LOG(LogDlgExportTwineCommandlet, Warning, TEXT("Dialogue = `%s` is not in the game directory, ignoring"), *DialoguePath);
			continue;
		}

		verify(DialoguePath.RemoveFromStart(TEXT("/Game")));
		const FString FileName = FPaths::GetBaseFilename(DialoguePath);
		const FString Directory = FPaths::GetPath(DialoguePath);

		FString FileSystemFilePath;
		if (bFlatten)
		{
			// Create in root of output directory

			// Make sure file does not exist
			FString FlattenedFileName = FileName;
			int32 CurrentTryIndex = 1;
			while (CreateFiles.Contains(FlattenedFileName) && CurrentTryIndex < 100)
			{
				FlattenedFileName = FString::Printf(TEXT("%s-%d"), *FileName, CurrentTryIndex);
				CurrentTryIndex++;
			}

			// Give up :(
			if (CreateFiles.Contains(FlattenedFileName))
			{
				UE_LOG(LogDlgExportTwineCommandlet, Warning, TEXT("Dialogue = `%s` could not generate unique flattened file, ignoring"), *DialoguePath);
				continue;
			}

			CreateFiles.Add(FlattenedFileName);
			FileSystemFilePath = OutputDirectory / FlattenedFileName + TEXT(".html");
		}
		else
		{
			// Ensure directory tree
			const FString FileSystemDirectoryPath = OutputDirectory / Directory;
			if (!PlatformFile.DirectoryExists(*FileSystemDirectoryPath) && PlatformFile.CreateDirectoryTree(*FileSystemDirectoryPath))
			{
				UE_LOG(LogDlgExportTwineCommandlet, Display, TEXT("Creating directory = `%s`"), *FileSystemDirectoryPath);
			}
			FileSystemFilePath = FileSystemDirectoryPath / FileName + TEXT(".html");
		}

		// Compute minimum graph node positions
		const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();
		MinimumGraphX = 0;
		MinimumGraphY = 0;

		// TODO: multiple start nodes?
		if (const UDialogueGraphNode* DialogueGraphNode = Cast<UDialogueGraphNode>(Dialogue->GetStartNodes()[0]->GetGraphNode()))
		{
			MinimumGraphX = FMath::Min(MinimumGraphX, DialogueGraphNode->NodePosX);
			MinimumGraphY = FMath::Min(MinimumGraphY, DialogueGraphNode->NodePosY);
		}
		for (const UDlgNode* Node : Nodes)
		{
			const UDialogueGraphNode* DialogueGraphNode = Cast<UDialogueGraphNode>(Node->GetGraphNode());
			if (DialogueGraphNode == nullptr)
			{
				continue;
			}

			MinimumGraphX = FMath::Min(MinimumGraphX, DialogueGraphNode->NodePosX);
			MinimumGraphY = FMath::Min(MinimumGraphY, DialogueGraphNode->NodePosY);
		}
		//UE_LOG(LogDlgExportTwineCommandlet, Verbose, TEXT("MinimumGraphX = %d, MinimumGraphY = %d"), MinimumGraphX, MinimumGraphY);

		// TODO: multiple start nodes?

		// Gather passages data
		CurrentNodesAreas.Empty();
		FString PassagesData;
		PassagesData += CreateTwinePassageDataFromNode(*Dialogue, *Dialogue->GetStartNodes()[0], INDEX_NONE) + TEXT("\n");

		// The rest of the nodes
		for (int32 NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++)
		{
			PassagesData += CreateTwinePassageDataFromNode(*Dialogue, *Nodes[NodeIndex], NodeIndex) + TEXT("\n");
		}

		// Export file
		const FString TwineFileContent = CreateTwineStoryData(Dialogue->GetDialogueName(), Dialogue->GetGUID(), INDEX_NONE, PassagesData);
		if (FFileHelper::SaveStringToFile(TwineFileContent, *FileSystemFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			UE_LOG(LogDlgExportTwineCommandlet, Display, TEXT("Writing file = `%s` for Dialogue = `%s` "), *FileSystemFilePath, *OriginalDialoguePath);
		}
		else
		{
			UE_LOG(LogDlgExportTwineCommandlet, Error, TEXT("FAILED to write file = `%s` for Dialogue = `%s`"), *FileSystemFilePath, *OriginalDialoguePath);
		}
	}

	return 0;
}


FString UDlgExportTwineCommandlet::CreateTwineStoryData(const FString& Name, const FGuid& DialogueGUID, int32 StartNodeIndex, const FString& PassagesData)
{
	static const FString Creator = TEXT("UE-NotYetDlgSystem");
	static const FString CreatorVersion = TEXT("5.0"); // TODO
	static constexpr int32 Zoom = 1;
	static const FString Format = TEXT("Harlowe");
	static const FString FormatVersion = TEXT("2.1.0");

	//const FGuid UUID = FGuid::NewGuid();
	return FString::Printf(
		TEXT("<tw-storydata name=\"%s\" startnode=\"%d\" creator=\"%s\" creator-version=\"%s\"")
		TEXT(" ifid=\"%s\" zoom=\"%d\" format=\"%s\" format-version=\"%s\" options=\"\" hidden>\n")

		TEXT("<style role=\"stylesheet\" id=\"twine-user-stylesheet\" type=\"text/twine-css\">%s</style>\n")
		TEXT("<script role=\"script\" id=\"twine-user-script\" type=\"text/twine-javascript\"></script>\n")

		// tags colors data
		TEXT("\n%s\n")
		// Special tag to identify the dialogue id
		//TEXT("<tw-passagedata pid=\"-1\" tags=\"\" name=\"DialogueGUID\" position=\"0,0\" size=\"10,10\">%s</tw-passagedata>\n")

		TEXT("%s\n")

		TEXT("</tw-storydata>"),
		*Name, StartNodeIndex + 2, *Creator, *CreatorVersion,
		*DialogueGUID.ToString(EGuidFormats::DigitsWithHyphens), Zoom, *Format, *FormatVersion,
		*CreateTwineCustomCss(),
		*CreateTwineTagColorsData(),
		*PassagesData
	);
}

bool UDlgExportTwineCommandlet::GetBoxThatConflicts(const FBox2D& Box, FBox2D& OutConflict)
{
	for (const FBox2D& CurrentBox : CurrentNodesAreas)
	{
		if (CurrentBox.Intersect(Box))
		{
			OutConflict = CurrentBox;
			return true;
		}
	}

	return false;
}

FIntPoint UDlgExportTwineCommandlet::GetNonConflictingPointFor(const FIntPoint& Point, const FIntPoint& Size, const FIntPoint& Padding)
{
	FVector2D MinVector(Point + Padding);
	FVector2D MaxVector(MinVector + Size);
	FBox2D NewBox(MinVector, MaxVector);

	FBox2D ConflictBox;
	while (GetBoxThatConflicts(NewBox, ConflictBox))
	{
		//UE_LOG(LogDlgExportTwineCommandlet, Warning, TEXT("Found conflict in rectangle: %s for Point: %s"), *ConflictRect.ToString(), *NewPoint.ToString());
		// Assume the curent box is a child, adjust
		FVector2D Center, Extent;
		ConflictBox.GetCenterAndExtents(Center, Extent);

		// Update on vertical
		MinVector.Y += Extent.Y / 2.f;
		MaxVector = MinVector + Size;
		NewBox = FBox2D(MinVector, MaxVector);
	}

	CurrentNodesAreas.Add(NewBox);
	return MinVector.IntPoint();
}

FString UDlgExportTwineCommandlet::CreateTwinePassageDataFromNode(const UDlgDialogue& Dialogue, const UDlgNode& Node, int32 NodeIndex)
{
	const UDialogueGraphNode* DialogueGraphNode = Cast<UDialogueGraphNode>(Node.GetGraphNode());
	if (DialogueGraphNode == nullptr)
	{
		UE_LOG(LogDlgExportTwineCommandlet, Warning, TEXT("Invalid UDialogueGraphNode for Node index = %d in Dialogue = `%s`. Ignoring."), NodeIndex, *Dialogue.GetPathName());
		return "";
	}
	const bool bIsRootNode = DialogueGraphNode->IsRootNode();

	const FString NodeName = GetNodeNameFromNode(Node, NodeIndex, bIsRootNode);
	FString Tags;
	FIntPoint Position = GraphNodeToTwineCanvas(DialogueGraphNode->NodePosX, DialogueGraphNode->NodePosY);

	// TODO fix this
	TSharedPtr<SGraphNode> NodeWidget = DialogueGraphNode->GetNodeWidget();
	FIntPoint Size = SizeLarge;
	if (NodeWidget.IsValid())
	{
		Size = FIntPoint(NodeWidget->GetDesiredSize().X, NodeWidget->GetDesiredSize().Y);
	}

	FString NodeContent;
	const FIntPoint Padding(20, 20);
	if (DialogueGraphNode->IsRootNode())
	{
		verify(NodeIndex == INDEX_NONE);
		Tags += TagNodeStart;
		Size = SizeSmall;
		Position = GetNonConflictingPointFor(Position, Size, Padding);

		NodeContent += CreateTwinePassageDataLinksFromEdges(Dialogue, Node.GetNodeChildren());
		return CreateTwinePassageData(NodeIndex, NodeName, Tags, Position, Size, NodeContent);
	}

	verify(NodeIndex >= 0);
	if (DialogueGraphNode->IsVirtualParentNode())
	{
		// Edges from this node do not matter
		Tags += TagNodeVirtualParent;
		Position = GetNonConflictingPointFor(Position, Size, Padding);
		//CurrentNodesAreas.Add(FIntRect(Position + Padding, Position + Size + Padding));

		const UDlgNode_Speech& NodeSpeech = DialogueGraphNode->GetDialogueNode<UDlgNode_Speech>();
		NodeContent += EscapeHtml(NodeSpeech.GetNodeUnformattedText().ToString());
		NodeContent += TEXT("\n\n\n") + CreateTwinePassageDataLinksFromEdges(Dialogue, Node.GetNodeChildren(), true);
		return CreateTwinePassageData(NodeIndex, NodeName, Tags, Position, Size, NodeContent);
	}
	if (DialogueGraphNode->IsSpeechNode())
	{
		Tags += TagNodeSpeech;
		Position = GetNonConflictingPointFor(Position, Size, Padding);
		//CurrentNodesAreas.Add(FIntRect(Position + Padding, Position + Size + Padding));

		const UDlgNode_Speech& NodeSpeech = DialogueGraphNode->GetDialogueNode<UDlgNode_Speech>();
		NodeContent += EscapeHtml(NodeSpeech.GetNodeUnformattedText().ToString());
		NodeContent += TEXT("\n\n\n") + CreateTwinePassageDataLinksFromEdges(Dialogue, Node.GetNodeChildren());
		return CreateTwinePassageData(NodeIndex, NodeName, Tags, Position, Size, NodeContent);
	}
	if (DialogueGraphNode->IsEndNode())
	{
		// Does not have any children/text
		Tags += TagNodeEnd;
		Size = SizeSmall;
		Position = GetNonConflictingPointFor(Position, Size, Padding);
		//CurrentNodesAreas.Add(FIntRect(Position + Padding, Position + Size + Padding));

		NodeContent += TEXT("END");
		return CreateTwinePassageData(NodeIndex, NodeName, Tags, Position, Size, NodeContent);
	}
	if (DialogueGraphNode->IsSelectorNode())
	{
		// Does not have any text and text for edges does not matter
		if (DialogueGraphNode->IsSelectorFirstNode())
		{
			Tags += TagNodeSelectorFirst;
		}
		if (DialogueGraphNode->IsSelectorRandomNode())
		{
			Tags += TagNodeSelectorRandom;
		}
		Size = SizeSmall;
		Position = GetNonConflictingPointFor(Position, Size, Padding);

		NodeContent += TEXT("SELECTOR\n");
		NodeContent += CreateTwinePassageDataLinksFromEdges(Dialogue, Node.GetNodeChildren(), true);
		return CreateTwinePassageData(NodeIndex, NodeName, Tags, Position, Size, NodeContent);
	}
	if (DialogueGraphNode->IsSpeechSequenceNode())
	{
		Tags += TagNodeSpeechSequence;
		Position = GetNonConflictingPointFor(Position, Size, Padding);

		const UDlgNode_SpeechSequence& NodeSpeechSequence = DialogueGraphNode->GetDialogueNode<UDlgNode_SpeechSequence>();

		// Fill sequence
		const TArray<FDlgSpeechSequenceEntry>& Sequence = NodeSpeechSequence.GetNodeSpeechSequence();
		for (int32 EntryIndex = 0; EntryIndex < Sequence.Num(); EntryIndex++)
		{
			const FDlgSpeechSequenceEntry& Entry = Sequence[EntryIndex];
			NodeContent += FString::Printf(
				TEXT("``Speaker:`` //%s//\n")
				TEXT("``Text:`` //%s//\n")
				TEXT("``EdgeText:`` //%s//\n"),
				*EscapeHtml(Entry.Speaker.ToString()),
				*EscapeHtml(Entry.Text.ToString()),
				*EscapeHtml(Entry.EdgeText.ToString())
			);

			if (EntryIndex != Sequence.Num() - 1)
			{
				NodeContent += TEXT("---\n");
			}
		}

		NodeContent += TEXT("\n\n\n") + CreateTwinePassageDataLinksFromEdges(Dialogue, Node.GetNodeChildren());
		return CreateTwinePassageData(NodeIndex, NodeName, Tags, Position, Size, NodeContent);
	}

	UE_LOG(LogDlgExportTwineCommandlet, Warning, TEXT("Node index = %d not handled in Dialogue = `%s`. Ignoring."), NodeIndex, *Dialogue.GetPathName());
	return "";
}

FString UDlgExportTwineCommandlet::GetNodeNameFromNode(const UDlgNode& Node, int32 NodeIndex, bool bIsRootNode)
{
	return FString::Printf(TEXT("%d. %s"), NodeIndex, bIsRootNode ? TEXT("START") : *Node.GetNodeParticipantName().ToString());
}

FString UDlgExportTwineCommandlet::CreateTwinePassageDataLinksFromEdges(const UDlgDialogue& Dialogue, const TArray<FDlgEdge>& Edges, bool bNoTextOnEdges)
{
	FString Links;
	const TArray<UDlgNode*>& Nodes = Dialogue.GetNodes();
	for (const FDlgEdge& Edge : Edges)
	{
		if (!Edge.IsValid())
		{
			continue;
		}
		if (!Nodes.IsValidIndex(Edge.TargetIndex))
		{
			UE_LOG(LogDlgExportTwineCommandlet, Warning, TEXT("Target index = %d not valid. Ignoring"), Edge.TargetIndex);
			continue;
		}

		FString EdgeText;
		if (bNoTextOnEdges || Edge.GetUnformattedText().IsEmpty())
		{
			EdgeText = FString::Printf(TEXT("~ignore~ To Node %d"), Edge.TargetIndex);
		}
		else
		{
			EdgeText = EscapeHtml(Edge.GetUnformattedText().ToString());
		}
		Links += FString::Printf(TEXT("[[%s|%s]]\n"), *EdgeText, *GetNodeNameFromNode(*Nodes[Edge.TargetIndex], Edge.TargetIndex, false));
	}
	Links.RemoveFromEnd(TEXT("\n"));
	return Links;
}


FString UDlgExportTwineCommandlet::CreateTwinePassageData(int32 Pid, const FString& Name, const FString& Tags, const FIntPoint& Position, const FIntPoint& Size, const FString& Content)
{
	return FString::Printf(
		TEXT("<tw-passagedata pid=\"%d\" name=\"%s\" tags=\"%s\" position=\"%d, %d\" size=\"%d, %d\">%s</tw-passagedata>"),
		Pid + 2, *Name, *Tags, Position.X, Position.Y, Size.X, Size.Y, *Content
	);
}

FString UDlgExportTwineCommandlet::CreateTwineCustomCss()
{
	return TEXT("#storyEditView.passage.tags div.cyan { background: #19e5e6; }");
}


FString UDlgExportTwineCommandlet::CreateTwineTagColorsData()
{
	InitTwinetagNodesColors();

	FString TagColorsString;
	for (const auto& Elem : TwineTagNodesColorsMap)
	{
		TagColorsString += FString::Printf(
			TEXT("<tw-tag name=\"%s\" color=\"%s\"></tw-tag>\n"),
			*Elem.Key, *Elem.Value
		);
	}

	return TagColorsString;
}

void UDlgExportTwineCommandlet::InitTwinetagNodesColors()
{
	if (TwineTagNodesColorsMap.Num() > 0)
	{
		return;
	}

	TwineTagNodesColorsMap.Add(TagNodeStart, TEXT("green"));
	TwineTagNodesColorsMap.Add(TagNodeEnd, TEXT("red"));
	TwineTagNodesColorsMap.Add(TagNodeVirtualParent, TEXT("blue"));
	TwineTagNodesColorsMap.Add(TagNodeSpeech, TEXT("blue"));
	TwineTagNodesColorsMap.Add(TagNodeSpeechSequence, TEXT("blue"));
	TwineTagNodesColorsMap.Add(TagNodeSelectorFirst, TEXT("purple"));
	TwineTagNodesColorsMap.Add(TagNodeSelectorRandom, TEXT("yellow"));
}
