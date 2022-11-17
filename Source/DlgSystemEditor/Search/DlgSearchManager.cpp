// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgSearchManager.h"

#include "Widgets/Docking/SDockTab.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DlgSearchUtilities.h"
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"
#include "EdGraphNode_Comment.h"
#include "Runtime/Launch/Resources/Version.h"

#include "DlgSystem/DlgDialogue.h"
#include "DlgSystem/DlgManager.h"
#include "DlgSystem/DlgHelper.h"
#include "SDlgFindInDialogues.h"
#include "DlgSystemEditor/Editor/Graph/DialogueGraph.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgSystemEditor/DlgStyle.h"
#include "DlgSystem/DlgConstants.h"

#define LOCTEXT_NAMESPACE "SDialogueBrowser"

FDlgSearchManager* FDlgSearchManager::Instance = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgSearchManager
FDlgSearchManager* FDlgSearchManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new Self();
	}

	return Instance;
}

FDlgSearchManager::FDlgSearchManager()
{
	// Create the Tab Ids
	for (int32 TabIdx = 0; TabIdx < NY_ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = FName(*FString::Printf(TEXT("GlobalNotYetDialogueFindResults_%02d"), TabIdx + 1));
		GlobalFindResultsTabIDs[TabIdx] = TabID;
	}
}

FDlgSearchManager::~FDlgSearchManager()
{
	UnInitialize();
}

bool FDlgSearchManager::QueryDlgTextArgument(
	const FDlgSearchFilter& SearchFilter,
	const FDlgTextArgument& InDlgTextArgument,
	const TSharedPtr<FDlgSearchResult>& OutParentNode,
	int32 ArgumentIndex
)
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
			LOCTEXT("DlgTextArgumentDisplayString", "TextArgument.DisplayString at index = {0}"),
			FText::AsNumber(ArgumentIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromString(InDlgTextArgument.DisplayString),
			Category,
			Category.ToString()
		);
	}

	// Test ParticipantName
	if (!InDlgTextArgument.ParticipantName.IsNone() &&
		InDlgTextArgument.ParticipantName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgTextArgumentParticipantName", "TextArgument.ParticipantName at index = {0}"),
			FText::AsNumber(ArgumentIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgTextArgument.ParticipantName),
			Category,
			Category.ToString()
		);
	}

	// Test VariableName
	if (!InDlgTextArgument.VariableName.IsNone() &&
		InDlgTextArgument.VariableName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgTextArgumentVariableName", "TextArgument.VariableName at index = {0}"),
			FText::AsNumber(ArgumentIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgTextArgument.VariableName),
			Category,
			Category.ToString()
		);
	}

	if (SearchFilter.bIncludeCustomObjectNames)
	{
		// Test CustomTextArgument
		FString FoundName;
		if (FDlgSearchUtilities::DoesObjectClassNameContainString(InDlgTextArgument.CustomTextArgument, SearchFilter.SearchString, FoundName))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgTextArgumentCustomTextArgument", "TextArgument.CustomTextArgument at index = {0}"),
				FText::AsNumber(ArgumentIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(FoundName),
				Category,
				Category.ToString()
			);
		}
	}

	return bContainsSearchString;
}

bool FDlgSearchManager::QueryDlgCondition(
	const FDlgSearchFilter& SearchFilter,
	const FDlgCondition& InDlgCondition,
	const TSharedPtr<FDlgSearchResult>& OutParentNode,
	int32 ConditionIndex,
	FName ConditionMemberName
)
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
		const FText Category = FText::Format(
			LOCTEXT("DlgConditionParticipantName", "{0}.ParticipantName at index = {1}"),
		 	 FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgCondition.ParticipantName),
			Category,
			Category.ToString()
		);
	}

	// Test CallBackName
	if (!InDlgCondition.CallbackName.IsNone() &&
		InDlgCondition.CallbackName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgConditionCallbackName", "{0}.CallbackName at index = {01}"),
			FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgCondition.CallbackName),
			Category,
			Category.ToString()
		);
	}

	// Test NameValue
	if (!InDlgCondition.NameValue.IsNone() &&
		InDlgCondition.NameValue.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgConditionNameValue", "{0}.NameValue at index = {1}"),
			FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgCondition.NameValue),
			Category,
			Category.ToString()
		);
	}

	// Test OtherParticipantName
	if (!InDlgCondition.OtherParticipantName.IsNone() &&
		InDlgCondition.OtherParticipantName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgConditionOtherParticipantName", "{0}.OtherParticipantName at index = {1}"),
			FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgCondition.OtherParticipantName),
			Category,
			Category.ToString()
		);
	}

	// Test OtherVariableName
	if (!InDlgCondition.OtherVariableName.IsNone() &&
		InDlgCondition.OtherVariableName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgConditionOtherVariableName", "{0}.OtherVariableName at index = {1}"),
			FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgCondition.OtherVariableName),
			Category,
			Category.ToString()
		);
	}

	if (SearchFilter.bIncludeCustomObjectNames)
	{
		// Test Custom Condition
		FString FoundName;
		if (FDlgSearchUtilities::DoesObjectClassNameContainString(InDlgCondition.CustomCondition, SearchFilter.SearchString, FoundName))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgConditionCustomCondition", "{0}.CustomCondition at index = {1}"),
				FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(FoundName),
				Category,
				Category.ToString()
			);
		}
	}

	if (SearchFilter.bIncludeNodeGUID)
	{
		// Test Node GUID
		FString FoundGUID;
		if (FDlgSearchUtilities::DoesGUIDContainString(InDlgCondition.GUID, SearchFilter.SearchString, FoundGUID))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgConditioGUID", "{0}.GUID at index = {1}"),
				FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(FoundGUID),
				Category,
				Category.ToString()
			);
		}
	}

	if (SearchFilter.bIncludeNumericalTypes)
	{
		// Test IntValue
		const FString IntValue = FString::FromInt(InDlgCondition.IntValue);
		if (IntValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgConditionIntValue", "{0}.IntValue at index = {1}"),
				FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(IntValue),
				Category,
				Category.ToString()
			);
		}

		// Test FloatValue
		const FString FloatValue = FString::SanitizeFloat(InDlgCondition.FloatValue);
		if (FloatValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgConditionFloatValue", "{0}.FloatValue at index = {1}"),
				FText::FromName(ConditionMemberName), FText::AsNumber(ConditionIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(FloatValue),
				Category,
				Category.ToString()
			);
		}
	}

	return bContainsSearchString;
}

bool FDlgSearchManager::QueryDlgEvent(
	const FDlgSearchFilter& SearchFilter,
	const FDlgEvent& InDlgEvent,
	const TSharedPtr<FDlgSearchResult>& OutParentNode,
	int32 EventIndex,
	FName EventMemberName
)
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
		const FText Category = FText::Format(
			LOCTEXT("DlgEventParticipantName", "{0}.ParticipantName at index = {1}"),
			FText::FromName(EventMemberName), FText::AsNumber(EventIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgEvent.ParticipantName),
			Category,
			Category.ToString()
		);
	}

	// Test EventName
	if (!InDlgEvent.EventName.IsNone() &&
		InDlgEvent.EventName.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgEventCallbackName", "{0}.EventName at index = {1}"),
			FText::FromName(EventMemberName), FText::AsNumber(EventIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgEvent.EventName),
			Category,
			Category.ToString()
		);
	}

	// Test NameValue
	if (!InDlgEvent.NameValue.IsNone() &&
		InDlgEvent.NameValue.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = FText::Format(
			LOCTEXT("DlgEventNameValue", "{0}.NameValue at index = {1}"),
			FText::FromName(EventMemberName), FText::AsNumber(EventIndex)
		);
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgEvent.NameValue),
			Category,
			Category.ToString()
		);
	}

	if (SearchFilter.bIncludeCustomObjectNames)
	{
		FString FoundName;
		if (FDlgSearchUtilities::DoesObjectClassNameContainString(InDlgEvent.CustomEvent, SearchFilter.SearchString, FoundName))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgEventCustomEvent", "{0}.CustomEvent at index = {1}"),
				FText::FromName(EventMemberName), FText::AsNumber(EventIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(FoundName),
				Category,
				Category.ToString()
			);
		}
	}

	if (SearchFilter.bIncludeNumericalTypes)
	{
		// Test IntValue
		const FString IntValue = FString::FromInt(InDlgEvent.IntValue);
		if (IntValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgEventIntValue", "{0}.IntValue at index = {1}"),
				FText::FromName(EventMemberName), FText::AsNumber(EventIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(IntValue),
				Category,
				Category.ToString()
			);
		}

		// Test FloatValue
		const FString FloatValue = FString::SanitizeFloat(InDlgEvent.FloatValue);
		if (FloatValue.Contains(SearchFilter.SearchString))
		{
			bContainsSearchString = true;
			const FText Category = FText::Format(
				LOCTEXT("DlgEventFloatValue", "{0}.FloatValue at index = {1}"),
				FText::FromName(EventMemberName), FText::AsNumber(EventIndex)
			);
			MakeChildTextNode(
				OutParentNode,
				FText::FromString(FloatValue),
				Category,
				Category.ToString()
			);
		}
	}

	return bContainsSearchString;
}

bool FDlgSearchManager::QueryDlgEdge(
	const FDlgSearchFilter& SearchFilter,
	const FDlgEdge& InDlgEdge,
	const TSharedPtr<FDlgSearchResult>& OutParentNode
)
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
		const FText Category = LOCTEXT("DlgEdgText", "Edge.Text");
		MakeChildTextNode(
			OutParentNode,
			InDlgEdge.GetUnformattedText(),
			Category,
			Category.ToString()
		);
	}
	// Test the Node Text Data
	if (SearchFilter.bIncludeTextLocalizationData)
	{
		bContainsSearchString = SearchForTextLocalizationData(
			OutParentNode,
			SearchFilter.SearchString,
			InDlgEdge.GetUnformattedText(),
			LOCTEXT("EdgeTextNamespaceName_Found", "Edge Text Namespace"), TEXT("Edge Text Localization Namespace"),
			LOCTEXT("EdgeTextKey_Found", "Edge Text Key"), TEXT("Edge Text Localization Key")
		) || bContainsSearchString;
	}

	// Test Condition
	for (int32 Index = 0, Num = InDlgEdge.Conditions.Num(); Index < Num; Index++)
	{
		bContainsSearchString = QueryDlgCondition(
			SearchFilter,
			InDlgEdge.Conditions[Index],
			OutParentNode,
			Index,
			TEXT("Condition")
		) || bContainsSearchString;
	}

	// Test SpeakerState
	if (!InDlgEdge.SpeakerState.IsNone() &&
		InDlgEdge.SpeakerState.ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		const FText Category = LOCTEXT("DlgEdgeSpeakerState", "Edge.SpeakerState");
		MakeChildTextNode(
			OutParentNode,
			FText::FromName(InDlgEdge.SpeakerState),
			Category,
			Category.ToString()
		);
	}

	// Test TextArguments
	const TArray<FDlgTextArgument>&  TextArguments = InDlgEdge.GetTextArguments();
	for (int32 Index = 0, Num = TextArguments.Num(); Index < Num; Index++)
	{
		bContainsSearchString = QueryDlgTextArgument(SearchFilter, TextArguments[Index], OutParentNode, Index) || bContainsSearchString;
	}

	return bContainsSearchString;
}

bool FDlgSearchManager::QueryGraphNode(
	const FDlgSearchFilter& SearchFilter,
	const UDialogueGraphNode* InGraphNode,
	const TSharedPtr<FDlgSearchResult>& OutParentNode
)
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
	const FText DisplayText = FText::Format(
		LOCTEXT("TreeGraphNodeCategory", "{0} Node at index {1}"),
		FText::FromString(NodeType), FText::AsNumber(NodeIndex)
	);
	TSharedPtr<FDlgSearchResult_GraphNode> TreeGraphNode = MakeShared<FDlgSearchResult_GraphNode>(DisplayText, OutParentNode);
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
			MakeChildTextNode(
				TreeGraphNode,
				FText::FromString(InGraphNode->NodeComment),
				LOCTEXT("NodeCommentKey", "Comment on Node"),
				TEXT("Comment on Node")
			);
		}
	}

	// Test the ParticipantName
	if (Node.GetNodeParticipantName().ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(
			TreeGraphNode,
			FText::FromName(Node.GetNodeParticipantName()),
			LOCTEXT("ParticipantNameKey", "Participant Name"),
			TEXT("Participant Name")
		);
	}

	// Test the Node text
	if (Node.GetNodeUnformattedText().ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(
			TreeGraphNode,
			Node.GetNodeUnformattedText(),
			LOCTEXT("DescriptionKey", "Text"),
			TEXT("Text")
		);
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
	const TArray<FDlgCondition>& EnterConditions = Node.GetNodeEnterConditions();
	for (int32 Index = 0, Num = EnterConditions.Num(); Index < Num; Index++)
	{
		bContainsSearchString = QueryDlgCondition(
			SearchFilter,
			EnterConditions[Index],
			TreeGraphNode,
			Index,
			TEXT("EnterCondition")
		) || bContainsSearchString;
	}

	// Test the EnterEvents
	const TArray<FDlgEvent>& EnterEvents = Node.GetNodeEnterEvents();
	for (int32 Index = 0, Num = EnterEvents.Num(); Index < Num; Index++)
	{
		bContainsSearchString = QueryDlgEvent(
			SearchFilter,
			EnterEvents[Index],
			TreeGraphNode,
			Index,
			TEXT("EnterEvent")
		) || bContainsSearchString;
	}

	// Test SpeakerState
	if (!Node.GetSpeakerState().IsNone() &&
		Node.GetSpeakerState().ToString().Contains(SearchFilter.SearchString))
	{
		bContainsSearchString = true;
		MakeChildTextNode(
			TreeGraphNode,
			FText::FromName(Node.GetSpeakerState()),
			LOCTEXT("SpeakerStateKey", "Speaker State"),
			TEXT("Speaker State")
		);
	}

	// Test TextArguments
	const TArray<FDlgTextArgument>& TextArguments = Node.GetTextArguments();
	for (int32 Index = 0, Num = TextArguments.Num(); Index < Num; Index++)
	{
		bContainsSearchString = QueryDlgTextArgument(SearchFilter, TextArguments[Index], TreeGraphNode, Index) || bContainsSearchString;
	}

	if (SearchFilter.bIncludeCustomObjectNames)
	{
		// Test Node Data
		FString FoundName;
		if (FDlgSearchUtilities::DoesObjectClassNameContainString(Node.GetNodeData(), SearchFilter.SearchString, FoundName))
		{
			bContainsSearchString = true;
			MakeChildTextNode(
				TreeGraphNode,
				FText::FromString(FoundName),
				LOCTEXT("NodeDataKey", "Node Data"),
				TEXT("Node Data")
			);
		}
	}

	if (SearchFilter.bIncludeNodeGUID)
	{
		// Test Node GUID
		FString FoundGUID;
		if (FDlgSearchUtilities::DoesGUIDContainString(Node.GetGUID(), SearchFilter.SearchString, FoundGUID))
		{
			bContainsSearchString = true;
			MakeChildTextNode(
				TreeGraphNode,
				FText::FromString(FoundGUID),
				LOCTEXT("NodeGUID", "Node GUID"),
				TEXT("Node GUID")
			);
		}
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

bool FDlgSearchManager::QueryEdgeNode(
	const FDlgSearchFilter& SearchFilter,
	const UDialogueGraphNode_Edge* InEdgeNode,
	const TSharedPtr<FDlgSearchResult>& OutParentNode
)
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
	TSharedPtr<FDlgSearchResult_EdgeNode> TreeEdgeNode = MakeShared<FDlgSearchResult_EdgeNode>(DisplayText, OutParentNode);
	TreeEdgeNode->SetCategory(DisplayText);
	TreeEdgeNode->SetEdgeNode(InEdgeNode);

	// Search in the DlgEdge
	const FDlgEdge& DialogueEdge = InEdgeNode->GetDialogueEdge();
	bContainsSearchString = QueryDlgEdge(SearchFilter, DialogueEdge, TreeEdgeNode) || bContainsSearchString;

	if (bContainsSearchString)
	{
		OutParentNode->AddChild(TreeEdgeNode);
	}

	return bContainsSearchString;
}

bool FDlgSearchManager::QueryCommentNode(
	const FDlgSearchFilter& SearchFilter,
	const UEdGraphNode_Comment* InCommentNode,
	const TSharedPtr<FDlgSearchResult>& OutParentNode
)
{
	if (!SearchFilter.bIncludeComments || SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid() || !IsValid(InCommentNode))
	{
		return false;
	}

	if (InCommentNode->NodeComment.Contains(SearchFilter.SearchString))
	{
		const FText Category = LOCTEXT("TreeNodeCommentCategory", "Comment Node");
		TSharedPtr<FDlgSearchResult_CommentNode> TreeCommentNode = MakeShared<FDlgSearchResult_CommentNode>(Category, OutParentNode);
		TreeCommentNode->SetCategory(Category);
		TreeCommentNode->SetCommentNode(InCommentNode);

		MakeChildTextNode(
			TreeCommentNode,
			FText::FromString(InCommentNode->NodeComment),
			Category,
			TEXT("")
		);

		OutParentNode->AddChild(TreeCommentNode);
		return true;
	}
	return false;
}

bool FDlgSearchManager::QuerySingleDialogue(
	const FDlgSearchFilter& SearchFilter,
	const UDlgDialogue* InDialogue,
	TSharedPtr<FDlgSearchResult>& OutParentNode
)
{
	if (SearchFilter.SearchString.IsEmpty() || !OutParentNode.IsValid() || !IsValid(InDialogue))
	{
		return false;
	}

	const UDialogueGraph* Graph = CastChecked<UDialogueGraph>(InDialogue->GetGraph());
	TSharedPtr<FDlgSearchResult_DialogueNode> TreeDialogueNode = MakeShared<FDlgSearchResult_DialogueNode>(
			FText::FromString(InDialogue->GetPathName()), OutParentNode
	);
	TreeDialogueNode->SetDialogue(InDialogue);

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
		bFoundInDialogue = bFoundInNode || bFoundInDialogue;
	}

	// Search for GUID
	if (SearchFilter.bIncludeDialogueGUID)
	{
		FString FoundGUID;
		if (FDlgSearchUtilities::DoesGUIDContainString(InDialogue->GetGUID(), SearchFilter.SearchString, FoundGUID))
		{
			bFoundInDialogue = true;
			MakeChildTextNode(
				TreeDialogueNode,
				FText::FromString(FoundGUID),
				LOCTEXT("DialogueGUID", "Dialogue GUID"),
				TEXT("Dialogue.GUID")
			);
		}
	}

	if (bFoundInDialogue)
	{
		OutParentNode->AddChild(TreeDialogueNode);
	}

	return bFoundInDialogue;
}

void FDlgSearchManager::QueryAllDialogues(
	const FDlgSearchFilter& SearchFilter,
	TSharedPtr<FDlgSearchResult>& OutParentNode
)
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

FText FDlgSearchManager::GetGlobalFindResultsTabLabel(int32 TabIdx)
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
		return FText::Format(
			LOCTEXT("GlobalFindResultsTabNameWithIndex", "Find in Dialogues {0}"),
			FText::AsNumber(TabIdx + 1)
		);
	}

	// No Number
	return LOCTEXT("GlobalFindResultsTabName", "Find in Dialogues");
}

void FDlgSearchManager::CloseGlobalFindResults(const TSharedRef<SDlgFindInDialogues>& FindResults)
{
	for (TWeakPtr<SDlgFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		// Remove it from the opened find results
		if (FindResultsPtr.Pin() == FindResults)
		{
			GlobalFindResultsWidgets.Remove(FindResultsPtr);
			break;
		}
	}
}

TSharedRef<SDockTab> FDlgSearchManager::SpawnGlobalFindResultsTab(const FSpawnTabArgs& SpawnTabArgs, int32 TabIdx)
{
	// Label is Dynamic depending on the number of global tabs
	TAttribute<FText> Label = TAttribute<FText>::Create(
		TAttribute<FText>::FGetter::CreateRaw(this, &Self::GetGlobalFindResultsTabLabel, TabIdx)
	);

	TSharedRef<SDockTab> NewTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab) // can be docked anywhere
		.Label(Label)
		.ToolTipText(LOCTEXT("GlobalFindResultsTabTooltip", "Search for a string in all Dialogue assets."));

	TSharedRef<SDlgFindInDialogues> FindResults = SNew(SDlgFindInDialogues)
		.bIsSearchWindow(false)
		.ContainingTab(NewTab);

	NewTab->SetContent(FindResults);
	GlobalFindResultsWidgets.Add(FindResults);

	return NewTab;
}

TSharedPtr<SDlgFindInDialogues> FDlgSearchManager::OpenGlobalFindResultsTab()
{
	TSet<FName> OpenGlobalTabIDs;

	// Check all opened global tabs
	for (TWeakPtr<SDlgFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SDlgFindInDialogues> FindResults = FindResultsPtr.Pin();
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
			TSharedPtr<SDockTab> NewTab = FDlgHelper::InvokeTab(FGlobalTabmanager::Get(), GlobalTabId);
			if (NewTab.IsValid())
			{
				return StaticCastSharedRef<SDlgFindInDialogues>(NewTab->GetContent());
			}
		}
	}

	return TSharedPtr<SDlgFindInDialogues>();
}

TSharedPtr<SDlgFindInDialogues> FDlgSearchManager::GetGlobalFindResults()
{
	// Find opened find tab
	TSharedPtr<SDlgFindInDialogues> FindResultsToUse;
	for (TWeakPtr<SDlgFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SDlgFindInDialogues> FindResults = FindResultsPtr.Pin();
		if (FindResults.IsValid())
		{
			FindResultsToUse = FindResults;
			break;
		}
	}

	if (FindResultsToUse.IsValid())
	{
		// found invoke it
		FDlgHelper::InvokeTab(FGlobalTabmanager::Get(), FindResultsToUse->GetHostTabId());
	}
	else
	{
		// not found, open a new tab.
		FindResultsToUse = OpenGlobalFindResultsTab();
	}

	return FindResultsToUse;
}

void FDlgSearchManager::EnableGlobalFindResults(TSharedPtr<FWorkspaceItem> ParentTabCategory)
{
	const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();

	// Register the spawners for all global Find Results tabs
	const FSlateIcon GlobalFindResultsIcon(FDlgStyle::GetStyleSetName(), FDlgStyle::PROPERTY_DialogueSearch_TabIcon);

	// Add the menu item
	if (!ParentTabCategory.IsValid())
	{
		ParentTabCategory = WorkspaceMenu::GetMenuStructure().GetToolsCategory();
	}

	GlobalFindResultsMenuItem = ParentTabCategory->AddGroup(
		LOCTEXT("WorkspaceMenu_GlobalFindResultsCategory", "Find in Dialogues"),
		LOCTEXT("GlobalFindResultsMenuTooltipText", "Find references to conditions, events, text and variables in all Dialogues."),
		GlobalFindResultsIcon,
		true
	);

	// Register tab spawners
	for (int32 TabIdx = 0; TabIdx < NY_ARRAY_COUNT(GlobalFindResultsTabIDs); TabIdx++)
	{
		const FName TabID = GlobalFindResultsTabIDs[TabIdx];

		// Tab not registered yet, good.
#if NY_ENGINE_VERSION >= 423
		if (!GlobalTabManager->HasTabSpawner(TabID))
#else
		if (!GlobalTabManager->CanSpawnTab(TabID))
#endif
		{
			const FText DisplayName = FText::Format(
				LOCTEXT("GlobalFindResultsDisplayName", "Find in Dialogues {0}"),
				FText::AsNumber(TabIdx + 1)
			);
			GlobalTabManager->RegisterNomadTabSpawner(
				TabID,
				FOnSpawnTab::CreateRaw(this, &Self::SpawnGlobalFindResultsTab, TabIdx))
				.SetDisplayName(DisplayName)
				.SetIcon(GlobalFindResultsIcon)
				.SetGroup(GlobalFindResultsMenuItem.ToSharedRef()
			);
		}
	}
}

void FDlgSearchManager::DisableGlobalFindResults()
{
	const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();

	// Close all Global Find Results tabs when turning the feature off, since these may not get closed along with the Blueprint Editor contexts above.
	for (TWeakPtr<SDlgFindInDialogues> FindResultsPtr : GlobalFindResultsWidgets)
	{
		TSharedPtr<SDlgFindInDialogues> FindResults = FindResultsPtr.Pin();
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

#if NY_ENGINE_VERSION >= 423
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

void FDlgSearchManager::Initialize(TSharedPtr<FWorkspaceItem> ParentTabCategory)
{
	// Must ensure we do not attempt to load the AssetRegistry Module while saving a package, however, if it is loaded already we can safely obtain it
	AssetRegistry = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(NAME_MODULE_AssetRegistry).Get();

	OnAssetAddedHandle = AssetRegistry->OnAssetAdded().AddRaw(this, &Self::HandleOnAssetAdded);
	OnAssetRemovedHandle = AssetRegistry->OnAssetRemoved().AddRaw(this, &Self::HandleOnAssetRemoved);
	OnAssetRenamedHandle = AssetRegistry->OnAssetRenamed().AddRaw(this, &Self::HandleOnAssetRenamed);

	if (AssetRegistry->IsLoadingAssets())
	{
		OnFilesLoadedHandle = AssetRegistry->OnFilesLoaded().AddRaw(this, &Self::HandleOnAssetRegistryFilesLoaded);
	}
	else
	{
		// Already loaded
		HandleOnAssetRegistryFilesLoaded();
	}
	OnAssetLoadedHandle = FCoreUObjectDelegates::OnAssetLoaded.AddRaw(this, &Self::HandleOnAssetLoaded);

	// Register global find results tabs
	EnableGlobalFindResults(ParentTabCategory);
}

void FDlgSearchManager::UnInitialize()
{
	if (AssetRegistry)
	{
		if (OnAssetAddedHandle.IsValid())
		{
			AssetRegistry->OnAssetAdded().Remove(OnAssetAddedHandle);
			OnAssetAddedHandle.Reset();
		}
		if (OnAssetRemovedHandle.IsValid())
		{
			AssetRegistry->OnAssetRemoved().Remove(OnAssetRemovedHandle);
			OnAssetRemovedHandle.Reset();
		}
		if (OnFilesLoadedHandle.IsValid())
		{
			AssetRegistry->OnFilesLoaded().Remove(OnFilesLoadedHandle);
			OnFilesLoadedHandle.Reset();
		}
		if (OnAssetRenamedHandle.IsValid())
		{
			AssetRegistry->OnAssetRenamed().Remove(OnAssetRenamedHandle);
			OnAssetRenamedHandle.Reset();
		}
	}

	if (OnAssetLoadedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnAssetLoaded.Remove(OnAssetLoadedHandle);
		OnAssetLoadedHandle.Reset();
	}

	// Shut down the global find results tab feature.
	DisableGlobalFindResults();
}

void FDlgSearchManager::BuildCache()
{
	// Difference between this and the UDlgManager::GetAllDialoguesFromMemory is that this loads all Dialogues
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
	// 	HandleOnAssetAdded(Asset);
	// }

	// We already loaded all Dialogues into memory in the StartupModule.
	for (UDlgDialogue* Dialogue : UDlgManager::GetAllDialoguesFromMemory())
	{
		FAssetData AssetData(Dialogue);
		HandleOnAssetAdded(AssetData);
	}
}

void FDlgSearchManager::HandleOnAssetAdded(const FAssetData& InAssetData)
{
	// Confirm that the Dialogue has not been added already, this can occur during duplication of Dialogues.
	const FDialogueSearchData* SearchDataPtr = SearchMap.Find(InAssetData.ToSoftObjectPath());
	if (SearchDataPtr != nullptr)
	{
		// Already exists
		return;
	}

	// Ignore other assets
	if (!InAssetData.GetClass() || !InAssetData.GetClass()->IsChildOf<UDlgDialogue>())
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
	SearchMap.Add(InAssetData.ToSoftObjectPath(), MoveTemp(SearchData));
}

void FDlgSearchManager::HandleOnAssetRemoved(const FAssetData& InAssetData)
{
	// TODO
}

void FDlgSearchManager::HandleOnAssetRenamed(const FAssetData& InAssetData, const FString& InOldName)
{
	// TODO
}

void FDlgSearchManager::HandleOnAssetLoaded(UObject* InAsset)
{

}

void FDlgSearchManager::HandleOnAssetRegistryFilesLoaded()
{
	// TODO Pause search if garbage collecting?
	FDlgEditorUtilities::LoadAllDialoguesAndCheckGUIDs();
	if (AssetRegistry)
	{
		// Do an immediate load of the cache to catch any Blueprints that were discovered by the asset registry before we initialized.
		BuildCache();
	}
}

#undef LOCTEXT_NAMESPACE
#undef NY_ARRAY_COUNT
