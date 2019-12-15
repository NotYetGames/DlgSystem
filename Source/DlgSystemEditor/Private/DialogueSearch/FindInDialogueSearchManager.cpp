// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "FindInDialogueSearchManager.h"

#include "Widgets/Docking/SDockTab.h"
#include "AssetRegistryModule.h"
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"
#include "EdGraphNode_Comment.h"

#include "DlgSystemEditorPrivatePCH.h"
#include "DlgDialogue.h"
#include "DlgManager.h"
#include "SFindInDialogues.h"
#include "DialogueEditor/Graph/DialogueGraph.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueStyle.h"

#define LOCTEXT_NAMESPACE "SDialogueBrowser"

#if ENGINE_MINOR_VERSION >= 24
	#define NY_ARRAY_COUNT UE_ARRAY_COUNT
#else
	#define NY_ARRAY_COUNT ARRAY_COUNT
#endif

FFindInDialogueSearchManager* FFindInDialogueSearchManager::Instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FFindInDialogueSearchManager
FFindInDialogueSearchManager* FFindInDialogueSearchManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new Self();
	}

	return Instance;
}

FFindInDialogueSearchManager::FFindInDialogueSearchManager()
{
	// Create the Tab Ids
	for (int32 TabIdx = 0; TabIdx < NY_ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = FName(*FString::Printf(TEXT("GlobalNotYetDialogueFindResults_%02d"), TabIdx + 1));
		GlobalFindResultsTabIDs[TabIdx] = TabID;
	}
}

FFindInDialogueSearchManager::~FFindInDialogueSearchManager()
{
	UnInitialize();
}

bool FFindInDialogueSearchManager::QueryDlgTextArgument(const FDialogueSearchFilter& SearchFilter, const FDlgTextArgument& InDlgTextArgument,
	const TSharedPtr<FFindInDialoguesResult>& OutParentNode, int32 ArgumentIndex)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid())
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Test DisplayString
	if (InDlgTextArgument.DisplayString.Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgTextArgumentDisplayString", "DlgTextArgument.DisplayString at index = {0}"), FText::AsNumber(ArgumentIndex));
		MakeChildTextNode(OutParentNode, FText::FromString(InDlgTextArgument.DisplayString), Category, Category.ToString());
	}

	// Test ParticipantName
	if (!InDlgTextArgument.ParticipantName.IsNone() &&
		InDlgTextArgument.ParticipantName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgTextArgumentParticipantName", "DlgTextArgument.ParticipantName at index = {0}"), FText::AsNumber(ArgumentIndex));
		MakeChildTextNode(OutParentNode, FText::FromName(InDlgTextArgument.ParticipantName), Category, Category.ToString());
	}

	// Test VariableName
	if (!InDlgTextArgument.VariableName.IsNone() &&
		InDlgTextArgument.VariableName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgTextArgumentVariableNamee", "DlgTextArgument.VariableName at index = {0}"), FText::AsNumber(ArgumentIndex));
		MakeChildTextNode(OutParentNode, FText::FromName(InDlgTextArgument.VariableName), Category, Category.ToString());
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryDlgCondition(const FDialogueSearchFilter& SearchFilter, const FDlgCondition& InDlgCondition,
													const TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid())
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Test ParticipantName
	if (!InDlgCondition.ParticipantName.IsNone() &&
		InDlgCondition.ParticipantName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgCondition.ParticipantName),
			LOCTEXT("DlgConditionParticipantName", "DlgCondition ParticipantName"),
			TEXT("DlgCondition.ParticipantName"));
	}

	// Test CallBackName
	if (!InDlgCondition.CallbackName.IsNone() &&
		InDlgCondition.CallbackName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgCondition.CallbackName),
			LOCTEXT("DlgConditionCallbackName", "DlgCondition CallbackName"),
			TEXT("DlgCondition.CallbackName"));
	}

	// Test NameValue
	if (!InDlgCondition.NameValue.IsNone() &&
		InDlgCondition.NameValue.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgCondition.NameValue),
			LOCTEXT("DlgConditionNameValue", "DlgCondition NameValue"),
			TEXT("DlgCondition.NameValue"));
	}

	// Test OtherParticipantName
	if (!InDlgCondition.OtherParticipantName.IsNone() &&
		InDlgCondition.OtherParticipantName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgCondition.OtherParticipantName),
			LOCTEXT("DlgConditionOtherParticipantName", "DlgCondition OtherParticipantName"),
			TEXT("DlgCondition.OtherParticipantName"));
	}

	// Test OtherVariableName
	if (!InDlgCondition.OtherVariableName.IsNone() &&
		InDlgCondition.OtherVariableName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgCondition.OtherVariableName),
			LOCTEXT("DlgConditionOtherVariableName", "DlgCondition OtherVariableName"),
			TEXT("DlgCondition.OtherVariableName"));
	}

	if (SearchFilter.bIncludeNumericalTypes)
	{
		// Test IntValue
		const FString IntValue = FString::FromInt(InDlgCondition.IntValue);
		if (IntValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			MakeChildTextNode(OutParentNode,
				FText::FromString(IntValue),
				LOCTEXT("DlgConditionIntValue", "DlgCondition IntValue"),
				TEXT("DlgCondition.IntValue"));
		}

		// Test FloatValue
		const FString FloatValue = FString::SanitizeFloat(InDlgCondition.FloatValue);
		if (FloatValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			MakeChildTextNode(OutParentNode,
				FText::FromString(FloatValue),
				LOCTEXT("DlgConditionFloatValue", "DlgCondition FloatValue"),
				TEXT("DlgCondition.FloatValue"));
		}
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryDlgEvent(const FDialogueSearchFilter& SearchFilter, const FDlgEvent& InDlgEvent,
												const TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid())
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Test ParticipantName
	if (!InDlgEvent.ParticipantName.IsNone() &&
		InDlgEvent.ParticipantName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgEvent.ParticipantName),
			LOCTEXT("DlgEventParticipantName", "DlgEvent ParticipantName"),
			TEXT("DlgEvent.ParticipantName"));
	}

	// Test EventName
	if (!InDlgEvent.EventName.IsNone() &&
		InDlgEvent.EventName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgEvent.EventName),
			LOCTEXT("DlgEventCallbackName", "DlgEvent EventName"),
			TEXT("DlgEvent.EventName"));
	}

	// Test NameValue
	if (!InDlgEvent.NameValue.IsNone() &&
		InDlgEvent.NameValue.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgEvent.NameValue),
			LOCTEXT("DlgEventNameValue", "DlgEvent NameValue"),
			TEXT("DlgEvent.NameValue"));
	}

	if (SearchFilter.bIncludeNumericalTypes)
	{
		// Test IntValue
		const FString IntValue = FString::FromInt(InDlgEvent.IntValue);
		if (IntValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			MakeChildTextNode(OutParentNode,
				FText::FromString(IntValue),
				LOCTEXT("DlgEventIntValue", "DlgEvent IntValue"),
				TEXT("DlgEvent.IntValue"));
		}

		// Test FloatValue
		const FString FloatValue = FString::SanitizeFloat(InDlgEvent.FloatValue);
		if (FloatValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			MakeChildTextNode(OutParentNode,
				FText::FromString(FloatValue),
				LOCTEXT("DlgEventFloatValue", "DlgEvent FloatValue"),
				TEXT("DlgEvent.FloatValue"));
		}
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryDlgEdge(const FDialogueSearchFilter& SearchFilter, const FDlgEdge& InDlgEdge,
												const TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid())
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Test Text
	if (InDlgEdge.GetUnformattedText().ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
						  InDlgEdge.GetUnformattedText(),
						  LOCTEXT("DlgEdgText", "DlgEdge Text"),
						  TEXT("DlgEdge.Text"));
	}
	// Test the Node Text Data
	if (SearchFilter.bIncludeTextLocalizationData)
	{
		bContainsSearchString = SearchForTextLocalizationData(
			OutParentNode,
			SearchFilter.SearchString, InDlgEdge.GetUnformattedText(),
			LOCTEXT("EdgeTextNamespaceName_Found", "Edge Text Namespace"), TEXT("Edge Text Localization Namespace"),
			LOCTEXT("EdgeTextKey_Found", "Edge Text Key"), TEXT("Edge Text Localization Key")
		) || bContainsSearchString;
	}

	// Test Condition
	for (const FDlgCondition& Condition : InDlgEdge.Conditions)
	{
		bContainsSearchString = QueryDlgCondition(SearchFilter, Condition, OutParentNode) || bContainsSearchString;
	}

	// Test SpeakerState
	if (!InDlgEdge.SpeakerState.IsNone() &&
		InDlgEdge.SpeakerState.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(OutParentNode,
			FText::FromName(InDlgEdge.SpeakerState),
			LOCTEXT("DlgEdgeSpeakerState", "DlgEdge SpeakerState"),
			TEXT("DlgEdge.SpeakerState"));
	}

	// Test TextArguments
	const TArray<FDlgTextArgument>&  TextArguments = InDlgEdge.GetTextArguments();
	for (int32 Index = 0, Num = TextArguments.Num(); Index < Num; Index++)
	{
		bContainsSearchString |= QueryDlgTextArgument(SearchFilter, TextArguments[Index], OutParentNode, Index);
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryGraphNode(const FDialogueSearchFilter& SearchFilter, const UDialogueGraphNode* InGraphNode,
												const TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid() || !IsValid(InGraphNode))
	{
		return false;
	}

	bool bContainsSearchString = false;
	const UDlgNode& Node = InGraphNode->GetDialogueNode();
	const int32 NodeIndex = InGraphNode->GetDialogueNodeIndex();
	const FString NodeType = Node.GetNodeTypeString();

	// Create the GraphNode Node
	const FText DisplayText = FText::Format(LOCTEXT("TreeGraphNodeCategory", "{0} Node at index {1}"),
										 FText::FromString(NodeType), FText::AsNumber(NodeIndex));
	TSharedPtr<FFindInDialoguesGraphNode> TreeGraphNode = MakeShared<FFindInDialoguesGraphNode>(DisplayText, OutParentNode);
	TreeGraphNode->SetCategory(FText::FromString(NodeType));
	TreeGraphNode->SetGraphNode(InGraphNode);

	// Test the NodeIndex
	if (SearchFilter.bIncludeIndices && !InGraphNode->IsRootNode())
	{
		// NOTE: We do not create another node, we just use the Node DisplayText as the search node.
		if (FString::FromInt(NodeIndex).Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
		}
	}

	// Test the Node Comment
	if (SearchFilter.bIncludeComments)
	{
		if (InGraphNode->NodeComment.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			MakeChildTextNode(TreeGraphNode,
				FText::FromString(InGraphNode->NodeComment),
				LOCTEXT("NodeCommentKey", "Comment on Node"),
				TEXT("Comment on Node"));
		}
	}

	// Test the ParticipantName
	if (Node.GetNodeParticipantName().ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(TreeGraphNode,
			FText::FromName(Node.GetNodeParticipantName()),
			LOCTEXT("ParticipantNameKey", "Participant Name"),
			TEXT("Participant Name"));
	}

	// Test the Node text
	if (Node.GetNodeUnformattedText().ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(TreeGraphNode, Node.GetNodeUnformattedText(), LOCTEXT("DescriptionKey", "Description"), TEXT("Description"));
	}
	// Test the Node Text Data
	if (SearchFilter.bIncludeTextLocalizationData)
	{
		bContainsSearchString = SearchForTextLocalizationData(
			TreeGraphNode,
			SearchFilter.SearchString, Node.GetNodeUnformattedText(),
			LOCTEXT("TextNamespaceName_Found", "Text Namespace"), TEXT("Text Localization Namespace"),
			LOCTEXT("TextKey_Found", "Text Key"), TEXT("Text Localization Key")
		) || bContainsSearchString;
	}

	// Test the EnterConditions
	for (const FDlgCondition& Condition : Node.GetNodeEnterConditions())
	{
		bContainsSearchString |= QueryDlgCondition(SearchFilter, Condition, TreeGraphNode);
	}

	// Test the EnterEvents
	for (const FDlgEvent& Event : Node.GetNodeEnterEvents())
	{
		bContainsSearchString |= QueryDlgEvent(SearchFilter, Event, TreeGraphNode);
	}

	// Test SpeakerState
	if (!Node.GetSpeakerState().IsNone() &&
		Node.GetSpeakerState().ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(TreeGraphNode,
			FText::FromName(Node.GetSpeakerState()),
			LOCTEXT("SpeakerStateKey", "Speaker State"),
			TEXT("Speaker State"));
	}

	// Test TextArguments
	const TArray<FDlgTextArgument>& TextArguments = Node.GetTextArguments();
	for (int32 Index = 0, Num = TextArguments.Num(); Index < Num; Index++)
	{
		bContainsSearchString |= QueryDlgTextArgument(SearchFilter, TextArguments[Index], TreeGraphNode, Index);
	}

	// Handle Speech sequences
	if (const UDlgNode_SpeechSequence* SpeechSequence = Cast<UDlgNode_SpeechSequence>(&Node))
	{
		const TArray<FDlgSpeechSequenceEntry>& SpeechSequenceArray = SpeechSequence->GetNodeSpeechSequence();
		for (int32 Index = 0, Num = SpeechSequenceArray.Num(); Index < Num; Index++)
		{
			const FDlgSpeechSequenceEntry& SequenceEntry = SpeechSequenceArray[Index];

			// Test Speaker
			if (SequenceEntry.Speaker.ToString().Contains(SearchFilter.SearchString))
			{
				bContainsSearchString = true;
				const FText Category = FText::Format(LOCTEXT("SequenceEntrySpeaker", "SequenceEntry.Speaker at index = {0}"), FText::AsNumber(Index));
				MakeChildTextNode(TreeGraphNode, FText::FromName(SequenceEntry.Speaker), Category, Category.ToString());
			}

			// Test Text Description
			const FText TextCategory = FText::Format(LOCTEXT("SequenceEntryText", "SequenceEntry.Text at index = {0}"), FText::AsNumber(Index));
			if (SequenceEntry.Text.ToString().Contains(SearchFilter.SearchString))
			{
				bContainsSearchString = true;
				MakeChildTextNode(TreeGraphNode, SequenceEntry.Text, TextCategory, TextCategory.ToString());
			}
			if (SearchFilter.bIncludeTextLocalizationData)
			{
				const FText NamespaceCategory = FText::FromString(TEXT("Namespace ") + TextCategory.ToString());
				const FText KeyCategory =  FText::FromString(TEXT("Key ") + TextCategory.ToString());
				bContainsSearchString = SearchForTextLocalizationData(
					TreeGraphNode,
					SearchFilter.SearchString, SequenceEntry.Text,
					NamespaceCategory, NamespaceCategory.ToString(),
					KeyCategory, KeyCategory.ToString()
				) || bContainsSearchString;
			}

			// Test EdgeText
			const FText EdgeTextCategory = FText::Format(LOCTEXT("SequenceEntryEdgeText", "SequenceEntry.EdgeText at index = {0}"), FText::AsNumber(Index));
			if (SequenceEntry.EdgeText.ToString().Contains(SearchFilter.SearchString))
			{
				bContainsSearchString = true;
				MakeChildTextNode(TreeGraphNode, SequenceEntry.EdgeText, EdgeTextCategory, EdgeTextCategory.ToString());
			}
			if (SearchFilter.bIncludeTextLocalizationData)
			{
				const FText NamespaceCategory = FText::FromString(TEXT("Namespace ") + EdgeTextCategory.ToString());
				const FText KeyCategory =  FText::FromString(TEXT("Key ") + EdgeTextCategory.ToString());
				bContainsSearchString = SearchForTextLocalizationData(
					TreeGraphNode,
					SearchFilter.SearchString, SequenceEntry.EdgeText,
					NamespaceCategory, NamespaceCategory.ToString(),
					KeyCategory, KeyCategory.ToString()
				) || bContainsSearchString;
			}

			// Test SpeakerState
			if (!SequenceEntry.SpeakerState.IsNone() &&
				SequenceEntry.SpeakerState.ToString().Contains(SearchFilter.SearchString))
			{
				bContainsSearchString = true;
				const FText Category = FText::Format(LOCTEXT("SequenceEntrySpeakerState", "SequenceEntry.SpeakerState at index = {0}"), FText::AsNumber(Index));
				MakeChildTextNode(TreeGraphNode, FText::FromName(SequenceEntry.SpeakerState), Category, Category.ToString());
			}
		}
	}

	if (bContainsSearchString)
	{
		OutParentNode->AddChild(TreeGraphNode);
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryEdgeNode(const FDialogueSearchFilter& SearchFilter, const UDialogueGraphNode_Edge* InEdgeNode,
	const TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid() || !IsValid(InEdgeNode))
	{
		return false;
	}
	bool bContainsSearchString = false;

	// Build up the Display Text
	int32 FromParent = -1;
	int32 ToChild = -1;
	if (InEdgeNode->HasParentNode())
	{
		FromParent = InEdgeNode->GetParentNode()->GetDialogueNodeIndex();
	}
	if (InEdgeNode->HasChildNode())
	{
		ToChild = InEdgeNode->GetChildNode()->GetDialogueNodeIndex();
	}
	const FText DisplayText = FText::Format(LOCTEXT("EdgeNodeDisplaytext", "Edge between {0} -> {1}"),
		FText::AsNumber(FromParent), FText::AsNumber(ToChild));
	TSharedPtr<FFindInDialoguesEdgeNode> TreeEdgeNode = MakeShared<FFindInDialoguesEdgeNode>(DisplayText, OutParentNode);
	TreeEdgeNode->SetCategory(DisplayText);
	TreeEdgeNode->SetEdgeNode(InEdgeNode);

	// Search in the DlgEdge
	const FDlgEdge& DialogueEdge = InEdgeNode->GetDialogueEdge();
	bContainsSearchString |= QueryDlgEdge(SearchFilter, DialogueEdge, TreeEdgeNode);

	if (bContainsSearchString)
	{
		OutParentNode->AddChild(TreeEdgeNode);
	}

	return bContainsSearchString;
}

bool FFindInDialogueSearchManager::QueryCommentNode(const FDialogueSearchFilter& SearchFilter, const UEdGraphNode_Comment* InCommentNode,
	const TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	if (!SearchFilter.bIncludeComments || SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid() || !IsValid(InCommentNode))
	{
		return false;
	}

	if (InCommentNode->NodeComment.Contains(SearchFilter.SearchString))
	{
		const FText Category = LOCTEXT("TreeNodeCommentCategory", "Comment Node");
		TSharedPtr<FFindInDialoguesCommentNode> TreeCommentNode = MakeShared<FFindInDialoguesCommentNode>(Category, OutParentNode);
		TreeCommentNode->SetCategory(Category);
		TreeCommentNode->SetCommentNode(InCommentNode);

		MakeChildTextNode(TreeCommentNode,
			FText::FromString(InCommentNode->NodeComment),
			Category,
			TEXT(""));

		OutParentNode->AddChild(TreeCommentNode);
		return true;
	}
	return false;
}

bool FFindInDialogueSearchManager::QuerySingleDialogue(const FDialogueSearchFilter& SearchFilter,
							const UDlgDialogue* InDialogue, TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid() || !IsValid(InDialogue))
	{
		return false;
	}

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(InDialogue->GetGraph());
	TSharedPtr<FFindInDialoguesDialogueNode> TreeDialogueNode = MakeShared<FFindInDialoguesDialogueNode>(
			FText::FromString(InDialogue->GetPathName()), OutParentNode
	);
	TreeDialogueNode->SetDialogue(InDialogue);

	// TODO node comments

	// Find in GraphNodes
	bool bFoundInDialogue = false;
	const TArray<UEdGraphNode*>& AllGraphNodes = Graph->GetAllGraphNodes();
	for (UEdGraphNode* Node : AllGraphNodes)
	{
		bool bFoundInNode = false;
		if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(Node))
		{
			bFoundInNode = QueryGraphNode(SearchFilter, GraphNode, TreeDialogueNode);
		}
		else if (UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(Node))
		{
			bFoundInNode = QueryEdgeNode(SearchFilter, EdgeNode, TreeDialogueNode);
		}
		else if (UEdGraphNode_Comment* CommentNode = Cast<UEdGraphNode_Comment>(Node))
		{
			bFoundInNode = QueryCommentNode(SearchFilter, CommentNode, TreeDialogueNode);
		}
		else
		{
			// ignore everything else
		}

		// Found at least one match in one of the nodes.
		bFoundInDialogue |= bFoundInNode;
	}

	// Search for GUID
	if (SearchFilter.bIncludeDialogueGUID)
	{
		const FString GuidToSearchFor = SearchFilter.SearchString.TrimStartAndEnd();
		const FGuid DialogueGuid = InDialogue->GetDlgGuid();

		// Test every possible format
		const TArray<FString> DialoguGuidStrings = {
			DialogueGuid.ToString(EGuidFormats::Digits),
			DialogueGuid.ToString(EGuidFormats::DigitsWithHyphens),
			DialogueGuid.ToString(EGuidFormats::DigitsWithHyphensInBraces),
			DialogueGuid.ToString(EGuidFormats::DigitsWithHyphensInParentheses),
			DialogueGuid.ToString(EGuidFormats::HexValuesInBraces),
			DialogueGuid.ToString(EGuidFormats::UniqueObjectGuid)
		};
		for (const FString& Guid : DialoguGuidStrings)
		{
			if (Guid.Contains(GuidToSearchFor))
			{
				bFoundInDialogue = true;
				MakeChildTextNode(TreeDialogueNode,
					FText::FromString(Guid),
					LOCTEXT("DlgGuid", "Dlg GUID"),
					TEXT("Dlg.Guid"));

				// Only one format is enough
				break;
			}
		}
	}

	if (bFoundInDialogue)
	{
		OutParentNode->AddChild(TreeDialogueNode);
	}

	return bFoundInDialogue;
}

void FFindInDialogueSearchManager::QueryAllDialogues(const FDialogueSearchFilter& SearchFilter,
	TSharedPtr<FFindInDialoguesResult>& OutParentNode)
{
	// Iterate over all cached dialogues
	for (auto& Elem : SearchMap)
	{
		const FDialogueSearchData& SearchData = Elem.Value;
		if (SearchData.Dialogue.IsValid())
		{
			QuerySingleDialogue(SearchFilter, SearchData.Dialogue.Get(), OutParentNode);
		}
	}
}

FText FFindInDialogueSearchManager::GetGlobalFindResultsTabLabel(int32 TabIdx)
{
	// Count the number of opened global Dialogues
	int32 NumOpenGlobalFindResultsTabs = 0;
	for (int32 i = GlobalFindResultsWidgets.Num() - 1; i >= 0; --i)
	{
		if (GlobalFindResultsWidgets[i].IsValid())
		{
			NumOpenGlobalFindResultsTabs++;
		}
		else
		{
			// Invalid :O remove it
			GlobalFindResultsWidgets.RemoveAt(i);
		}
	}

	if (NumOpenGlobalFindResultsTabs > 1 || TabIdx > 0)
	{
		// Format TabIdx + 1
		return FText::Format(LOCTEXT("GlobalFindResultsTabNameWithIndex", "Find in Dialogues {0}"),
							FText::AsNumber(TabIdx + 1));
	}

	// No Number
	return LOCTEXT("GlobalFindResultsTabName", "Find in Dialogues");
}

void FFindInDialogueSearchManager::CloseGlobalFindResults(const TSharedRef<SFindInDialogues>& FindResults)
{
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		// Remove it from the opened find results
		if (FindResultsPtr.Pin() == FindResults)
		{
			GlobalFindResultsWidgets.Remove(FindResultsPtr);
			break;
		}
	}
}

TSharedRef<SDockTab> FFindInDialogueSearchManager::SpawnGlobalFindResultsTab(const FSpawnTabArgs& SpawnTabArgs, int32 TabIdx)
{
	// Label is Dynamic depending on the number of global tabs
	TAttribute<FText> Label = TAttribute<FText>::Create(
		TAttribute<FText>::FGetter::CreateRaw(this, &Self::GetGlobalFindResultsTabLabel, TabIdx));

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab) // can be docked anywhere
		.Label(Label)
		.ToolTipText(LOCTEXT("GlobalFindResultsTabTooltip", "Search for a string in all Dialogue assets."));

	TSharedRef<SFindInDialogues> FindResults = SNew(SFindInDialogues)
		.bIsSearchWindow(false)
		.ContainingTab(NewTab);

	NewTab->SetContent(FindResults);
	GlobalFindResultsWidgets.Add(FindResults);

	return NewTab;
}

TSharedPtr<SFindInDialogues> FFindInDialogueSearchManager::OpenGlobalFindResultsTab()
{
	TSet<FName> OpenGlobalTabIDs;

	// Check all opened global tabs
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SFindInDialogues> FindResults = FindResultsPtr.Pin();
		if (FindResults.IsValid())
		{
			OpenGlobalTabIDs.Add(FindResults->GetHostTabId());
		}
	}

	// Find the unopened tab, that we can open
	for (int32 Idx = 0; Idx < NY_ARRAY_COUNT(GlobalFindResultsTabIDs); ++Idx)
	{
		const FName GlobalTabId = GlobalFindResultsTabIDs[Idx];
		if (!OpenGlobalTabIDs.Contains(GlobalTabId))
		{
			// GlobalTabId is not opened, open it now
			TSharedRef<SDockTab> NewTab = FGlobalTabmanager::Get()->InvokeTab(GlobalTabId);
			return StaticCastSharedRef<SFindInDialogues>(NewTab->GetContent());
		}
	}

	return TSharedPtr<SFindInDialogues>();
}

TSharedPtr<SFindInDialogues> FFindInDialogueSearchManager::GetGlobalFindResults()
{
	// Find opened find tab
	TSharedPtr<SFindInDialogues> FindResultsToUse;
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SFindInDialogues> FindResults = FindResultsPtr.Pin();
		if (FindResults.IsValid())
		{
			FindResultsToUse = FindResults;
			break;
		}
	}

	if (FindResultsToUse.IsValid())
	{
		// found invoke it
		FGlobalTabmanager::Get()->InvokeTab(FindResultsToUse->GetHostTabId());
	}
	else
	{
		// not found, open a new tab.
		FindResultsToUse = OpenGlobalFindResultsTab();
	}

	return FindResultsToUse;
}

void FFindInDialogueSearchManager::EnableGlobalFindResults(TSharedPtr<FWorkspaceItem> ParentTabCategory)
{
	const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();

	// Register the spawners for all global Find Results tabs
	const FSlateIcon GlobalFindResultsIcon(FDialogueStyle::GetStyleSetName(), FDialogueStyle::PROPERTY_FindDialogueIcon);

	// Add the menu item
	if (!ParentTabCategory.IsValid())
	{
		ParentTabCategory = WorkspaceMenu::GetMenuStructure().GetToolsCategory();
	}

	GlobalFindResultsMenuItem = ParentTabCategory->AddGroup(
		LOCTEXT("WorkspaceMenu_GlobalFindResultsCategory", "Find in Dialogues"),
		LOCTEXT("GlobalFindResultsMenuTooltipText", "Find references to conditions, events, text and variables in all Dialogues."),
		GlobalFindResultsIcon,
		true);

	// Register tab spawners
	for (int32 TabIdx = 0; TabIdx < NY_ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = GlobalFindResultsTabIDs[TabIdx];

		// Tab not registered yet, good.
#if ENGINE_MINOR_VERSION >= 23
		if (!GlobalTabManager->HasTabSpawner(TabID))
#else
		if (!GlobalTabManager->CanSpawnTab(TabID))
#endif
		{
			const FText DisplayName = FText::Format(LOCTEXT("GlobalFindResultsDisplayName", "Find in Dialogues {0}"),
													FText::AsNumber(TabIdx + 1));
			GlobalTabManager->RegisterNomadTabSpawner(TabID, FOnSpawnTab::CreateRaw(this, &Self::SpawnGlobalFindResultsTab, TabIdx))
				.SetDisplayName(DisplayName)
				.SetIcon(GlobalFindResultsIcon)
				.SetGroup(GlobalFindResultsMenuItem.ToSharedRef());
		}
	}
}

void FFindInDialogueSearchManager::DisableGlobalFindResults()
{
	const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();

	// Close all Global Find Results tabs when turning the feature off, since these may not get closed along with the Blueprint Editor contexts above.
	for (TWeakPtr<SFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SFindInDialogues> FindResults = FindResultsPtr.Pin();
		if (FindResults.IsValid())
		{
			FindResults->CloseHostTab();
		}
	}
	GlobalFindResultsWidgets.Empty();

	// Unregister tab spawners
	for (int32 TabIdx = 0; TabIdx < NY_ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = GlobalFindResultsTabIDs[TabIdx];

#if ENGINE_MINOR_VERSION >= 23
		if (!GlobalTabManager->HasTabSpawner(TabID))
#else
		if (!GlobalTabManager->CanSpawnTab(TabID))
#endif
		{
			GlobalTabManager->UnregisterNomadTabSpawner(TabID);
		}
	}

	// Remove the menu item
	if (GlobalFindResultsMenuItem.IsValid())
	{
		WorkspaceMenu::GetMenuStructure().GetToolsCategory()->RemoveItem(GlobalFindResultsMenuItem.ToSharedRef());
		GlobalFindResultsMenuItem.Reset();
	}
}

void FFindInDialogueSearchManager::Initialize(TSharedPtr<FWorkspaceItem> ParentTabCategory)
{
	// Must ensure we do not attempt to load the AssetRegistry Module while saving a package, however, if it is loaded already we can safely obtain it
	if (!GIsSavingPackage || (GIsSavingPackage && FModuleManager::Get().IsModuleLoaded(NAME_MODULE_AssetRegistry)))
	{
		AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(NAME_MODULE_AssetRegistry);
		AssetRegistryModule->Get().OnAssetAdded().AddRaw(this, &Self::HandleAssetAdded);
		AssetRegistryModule->Get().OnAssetRemoved().AddRaw(this, &Self::HandleAssetRemoved);
		AssetRegistryModule->Get().OnAssetRenamed().AddRaw(this, &Self::HandleAssetRenamed);
	}
	else
	{
		// Log a warning to inform the Asset Registry could not be initialized when FiD initialized due to saving package
		// The Asset Registry should be initialized before Find-in-Dialogues, or FiD should be explicitly initialized during a safe time
		// This message will not appear in commandlets because most commandlets do not care. If a search query is made, further warnings will be produced even in commandlets.
		if (!IsRunningCommandlet())
		{
			UE_LOG(LogDlgSystemEditor,
				Warning,
				TEXT("Find-in-Dialogues could not pre-cache all unloaded Dialogues due to the Asset Registry module being unable to initialize because a package is currently being saved. Pre-cache will not be reattempted!"));
		}
	}
	FCoreUObjectDelegates::OnAssetLoaded.AddRaw(this, &Self::HandleAssetLoaded);

	// TODO Pause search if garbage collecting?
	if (!GIsSavingPackage && AssetRegistryModule)
	{
		// Do an immediate load of the cache to catch any Blueprints that were discovered by the asset registry before we initialized.
		BuildCache();
	}

	// Register global find results tabs
	EnableGlobalFindResults(ParentTabCategory);
}

void FFindInDialogueSearchManager::UnInitialize()
{
	if (AssetRegistryModule)
	{
		AssetRegistryModule->Get().OnAssetAdded().RemoveAll(this);
		AssetRegistryModule->Get().OnAssetRemoved().RemoveAll(this);
		AssetRegistryModule->Get().OnAssetRenamed().RemoveAll(this);
	}
	FCoreUObjectDelegates::OnAssetLoaded.RemoveAll(this);

	// Shut down the global find results tab feature.
	DisableGlobalFindResults();
}

void FFindInDialogueSearchManager::BuildCache()
{
	// Difference between this and the UDlgManger::GetAllDialoguesFromMemory is that this loads all Dialogues
	// even those that are not loaded into memory.
	// TODO this seems slow :(
	// AssetRegistryModule = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	// FARFilter ClassFilter;
	// ClassFilter.bRecursiveClasses = true;
	// ClassFilter.ClassNames.Add(UDlgDialogue::StaticClass()->GetFName());
	// TArray<FAssetData> DialogueAssets;
	// AssetRegistryModule->Get().GetAssets(ClassFilter, DialogueAssets);
	// for (FAssetData& Asset : DialogueAssets)
	// {
	// 	HandleAssetAdded(Asset);
	// }

	// We already loaded all Dialogues into memory in the StartupModule.
	for (UDlgDialogue* Dialogue : UDlgManager::GetAllDialoguesFromMemory())
	{
		FAssetData AssetData(Dialogue);
		HandleAssetAdded(AssetData);
	}
}

void FFindInDialogueSearchManager::HandleAssetAdded(const FAssetData& InAssetData)
{
	// Confirm that the Dialogue has not been added already, this can occur during duplication of Dialogues.
	FDialogueSearchData* SearchDataPtr = SearchMap.Find(InAssetData.ObjectPath);
	if (SearchDataPtr != nullptr)
	{
		// Already exists
		return;
	}

	// Ignore other assets
	if (InAssetData.GetClass() && !InAssetData.GetClass()->IsChildOf<UDlgDialogue>())
	{
		return;
	}

	// Load the Dialogue
	UDlgDialogue* Dialogue = Cast<UDlgDialogue>(InAssetData.GetAsset());
	if (!IsValid(Dialogue))
	{
		return;
	}

	// Add to the loaded cached map
	FDialogueSearchData SearchData;
	SearchData.Dialogue = Dialogue;
	SearchMap.Add(InAssetData.ObjectPath, MoveTemp(SearchData));
}

void FFindInDialogueSearchManager::HandleAssetRemoved(const FAssetData& InAssetData)
{
	// TODO
}

void FFindInDialogueSearchManager::HandleAssetRenamed(const FAssetData& InAssetData, const FString& InOldName)
{
	// TODO
}

void FFindInDialogueSearchManager::HandleAssetLoaded(UObject* InAsset)
{

}

#undef LOCTEXT_NAMESPACE
#undef NY_ARRAY_COUNT
