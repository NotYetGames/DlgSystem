// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgManager.h"

#include "UObject/UObjectIterator.h"
#include "Engine/ObjectLibrary.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/Blueprint.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"

#include "IDlgSystemModule.h"
#include "DlgConstants.h"
#include "DlgDialogueParticipant.h"
#include "DlgDialogue.h"
#include "DlgMemory.h"
#include "DlgContext.h"
#include "Logging/DlgLogger.h"
#include "DlgHelper.h"
#include "NYReflectionHelper.h"

TWeakObjectPtr<const UObject> UDlgManager::UserWorldContextObjectPtr = nullptr;

bool UDlgManager::bCalledLoadAllDialoguesIntoMemory = false;;

UDlgContext* UDlgManager::StartDialogueWithDefaultParticipants(UObject* WorldContextObject, UDlgDialogue* Dialogue)
{
	if (!IsValid(Dialogue))
	{
		FDlgLogger::Get().Error(TEXT("StartDialogueWithDefaultParticipants - FAILED to start dialogue because the Dialogue is INVALID (is nullptr)!"));
		return nullptr;
	}

	// Create empty map of participants we need
	TSet<FName> ParticipantSet = Dialogue->GetParticipantNames();
	TArray<UObject*> Participants;

	// Maps from Participant Name => Objects that have that participant name
	TMap<FName, TArray<UObject*>> ObjectMap;
	for (const FName& Name : ParticipantSet)
	{
		ObjectMap.Add(Name, {});
	}

	// Gather all objects that have our participant name
	for (UObject* Participant : GetObjectsWithDialogueParticipantInterface(WorldContextObject))
	{
		const FName ParticipantName = IDlgDialogueParticipant::Execute_GetParticipantName(Participant);
		if (ObjectMap.Contains(ParticipantName))
		{
			ObjectMap[ParticipantName].AddUnique(Participant);
			Participants.AddUnique(Participant);
		}
	}

	// Find the missing names and the duplicate names
	TArray<FString> MissingNames;
	TArray<FString> DuplicatedNames;
	for (const auto& Pair : ObjectMap)
	{
		const FName ParticipantName = Pair.Key;
		const TArray<UObject*>& Objects = Pair.Value;

		if (Objects.Num() == 0)
		{
			MissingNames.Add(ParticipantName.ToString());
		}
		else if (Objects.Num() > 1)
		{
			for (UObject* Obj : Objects)
			{
				DuplicatedNames.Add(Obj->GetName() + "(" + ParticipantName.ToString() + ")");
			}
		}
	}

	if (MissingNames.Num() > 0)
	{
		const FString NameList = FString::Join(MissingNames, TEXT(", "));
		FDlgLogger::Get().Errorf(
			TEXT("StartDialogueWithDefaultParticipants - FAILED Dialogue = `%s`, the system FAILED to find the following Participant(s): %s"),
			*Dialogue->GetName(), *NameList
		);
	}

	if (DuplicatedNames.Num() > 0)
	{
		const FString NameList = FString::Join(DuplicatedNames, TEXT(", "));
		FDlgLogger::Get().Errorf(
			TEXT("StartDialogueWithDefaultParticipants - FAILED for Dialogue = `%s`, the system found multiple participants using the same name: %s"),
			*Dialogue->GetName(), *NameList
		);
	}

	if (MissingNames.Num() > 0 || DuplicatedNames.Num() > 0)
	{
		return nullptr;
	}

	return StartDialogueWithContext(TEXT("StartDialogueWithDefaultParticipants"), Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogueWithContext(const FString& ContextString, UDlgDialogue* Dialogue, const TArray<UObject*>& Participants)
{
	const FString ContextMessage = ContextString.IsEmpty()
		? FString::Printf(TEXT("StartDialogue"))
		: FString::Printf(TEXT("%s - StartDialogue"), *ContextString);

	TMap<FName, UObject*> ParticipantBinding;
	if (!UDlgContext::ConvertArrayOfParticipantsToMap(ContextMessage, Dialogue, Participants, ParticipantBinding))
	{
		return nullptr;
	}

	auto* Context = NewObject<UDlgContext>(Participants[0], UDlgContext::StaticClass());
	if (Context->StartWithContext(ContextMessage, Dialogue, ParticipantBinding))
	{
		return Context;
	}

	return nullptr;
}

bool UDlgManager::CanStartDialogue(UDlgDialogue* Dialogue, UPARAM(ref)const TArray<UObject*>& Participants)
{
	TMap<FName, UObject*> ParticipantBinding;
	if (!UDlgContext::ConvertArrayOfParticipantsToMap(TEXT("CanStartDialogue"), Dialogue, Participants, ParticipantBinding, false))
	{
		return false;
	}

	return UDlgContext::CanBeStarted(Dialogue, ParticipantBinding);
}

UDlgContext* UDlgManager::ResumeDialogueFromNodeIndex(
	UDlgDialogue* Dialogue,
	UPARAM(ref)const TArray<UObject*>& Participants,
	int32 StartNodeIndex,
	const TSet<int32>& AlreadyVisitedNodes,
	bool bFireEnterEvents
)
{
	const FString ContextMessage = TEXT("ResumeDialogueFromNodeIndex");
	TMap<FName, UObject*> ParticipantBinding;
	if (!UDlgContext::ConvertArrayOfParticipantsToMap(ContextMessage, Dialogue, Participants, ParticipantBinding))
	{
		return nullptr;
	}

	auto* Context = NewObject<UDlgContext>(Participants[0], UDlgContext::StaticClass());
	FDlgHistory History;
	History.VisitedNodeIndices = AlreadyVisitedNodes;
	if (Context->StartWithContextFromNodeIndex(ContextMessage, Dialogue, ParticipantBinding, StartNodeIndex, History, bFireEnterEvents))
	{
		return Context;
	}

	return nullptr;
}

UDlgContext* UDlgManager::ResumeDialogueFromNodeGUID(
	UDlgDialogue* Dialogue,
	UPARAM(ref)const TArray<UObject*>& Participants,
	const FGuid& StartNodeGUID,
	const TSet<FGuid>& AlreadyVisitedNodes,
	bool bFireEnterEvents
)
{
	const FString ContextMessage = TEXT("ResumeDialogueFromNodeGUID");
	TMap<FName, UObject*> ParticipantBinding;
	if (!UDlgContext::ConvertArrayOfParticipantsToMap(ContextMessage, Dialogue, Participants, ParticipantBinding))
	{
		return nullptr;
	}

	auto* Context = NewObject<UDlgContext>(Participants[0], UDlgContext::StaticClass());
	FDlgHistory History;
	History.VisitedNodeGUIDs = AlreadyVisitedNodes;
	if (Context->StartWithContextFromNodeGUID(ContextMessage, Dialogue, ParticipantBinding, StartNodeGUID, History, bFireEnterEvents))
	{
		return Context;
	}

	return nullptr;
}

UDlgContext* UDlgManager::StartMonologue(UDlgDialogue* Dialogue, UObject* Participant)
{
	TArray<UObject*> Participants;
	Participants.Add(Participant);
	return StartDialogueWithContext(TEXT("StartMonologue"), Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue2(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1)
{
	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	return StartDialogueWithContext(TEXT("StartDialogue2"), Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue3(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2)
{
	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	Participants.Add(Participant2);
	return StartDialogueWithContext(TEXT("StartDialogue3"), Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue4(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2, UObject* Participant3)
{
	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	Participants.Add(Participant2);
	Participants.Add(Participant3);

	return StartDialogueWithContext(TEXT("StartDialogue4"), Dialogue, Participants);
}

int32 UDlgManager::LoadAllDialoguesIntoMemory(bool bAsync)
{
	bCalledLoadAllDialoguesIntoMemory = true;

	// NOTE: All paths must NOT have the forward slash "/" at the end.
	// If they do, then this won't load Dialogues that are located in the Content root directory
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UDlgDialogue::StaticClass(), false, GIsEditor);
	TArray<FString> PathsToSearch = { TEXT("/Game") };
	ObjectLibrary->AddToRoot();

	// Add the current plugin dir
	// TODO maybe add all the non engine plugin paths? IPluginManager::Get().GetEnabledPlugins()
	const TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(DIALOGUE_SYSTEM_PLUGIN_NAME.ToString());
	if (ThisPlugin.IsValid())
	{
		FString PluginPath = ThisPlugin->GetMountedAssetPath();
		// See NOTE above
		PluginPath.RemoveFromEnd(TEXT("/"));
		PathsToSearch.Add(PluginPath);
	}

	const bool bForceSynchronousScan = !bAsync;
	const int32 Count = ObjectLibrary->LoadAssetDataFromPaths(PathsToSearch, bForceSynchronousScan);
	ObjectLibrary->LoadAssetsFromAssetData();
	ObjectLibrary->RemoveFromRoot();

	return Count;
}

TArray<UDlgDialogue*> UDlgManager::GetAllDialoguesFromMemory()
{
#if WITH_EDITOR
	// Hmm, something is wrong
	if (!bCalledLoadAllDialoguesIntoMemory)
	{
		LoadAllDialoguesIntoMemory(false);
	}
// 	check(bCalledLoadAllDialoguesIntoMemory);
#endif

	TArray<UDlgDialogue*> Array;
	for (TObjectIterator<UDlgDialogue> Itr; Itr; ++Itr)
	{
		UDlgDialogue* Dialogue = *Itr;
		if (IsValid(Dialogue))
		{
			Array.Add(Dialogue);
		}
	}
	return Array;
}

TArray<TWeakObjectPtr<AActor>> UDlgManager::GetAllWeakActorsWithDialogueParticipantInterface(UWorld* World)
{
	TArray<TWeakObjectPtr<AActor>> Array;
	for (TActorIterator<AActor> Itr(World); Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		if (IsValid(Actor) && !IsValid(Actor) && Actor->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
		{
			Array.Add(Actor);
		}
	}
	return Array;
}

TArray<UObject*> UDlgManager::GetObjectsWithDialogueParticipantInterface(UObject* WorldContextObject)
{
	TArray<UObject*> Array;
	if (!WorldContextObject)
		return Array;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		// TObjectIterator has some weird ghost objects in editor, I failed to find a way to validate them
		// Instead of this ActorIterate is used and the properties inside the actors are examined in a recursive way
		// Containers are not supported yet
		TSet<UObject*> VisitedSet;
		for (TActorIterator<AActor> Itr(World); Itr; ++Itr)
		{
			GatherParticipantsRecursive(*Itr, Array, VisitedSet);
		}
	}

	// TArray<UObject*> Array2;
	// for (TObjectIterator<UObject> Itr; Itr; ++Itr)
	// {
	// 	UObject* Object = *Itr;
	// 	if (IsValid(Object)
	// 		&& !Object->IsPendingKill()
	// 		&& Object->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
	// 		//&& IDlgDialogueParticipant::Execute_GetParticipantName(Object) != NAME_None)
	// 	{
	// 		if (Object->HasAllFlags(RF_Transient | RF_Transactional) || !Object->HasAnyFlags(RF_Transient) )
	// 		{
	// 			Array2.AddUnique(Object);
	// 		}
	// 	}
	// }

	return Array;
}

TMap<FName, FDlgObjectsArray> UDlgManager::GetObjectsMapWithDialogueParticipantInterface(UObject* WorldContextObject)
{
	// Maps from Participant Name => Objects that have that participant name
	TMap<FName, FDlgObjectsArray> ObjectsMap;
	for (UObject* Participant : GetObjectsWithDialogueParticipantInterface(WorldContextObject))
	{
		const FName ParticipantName = IDlgDialogueParticipant::Execute_GetParticipantName(Participant);
		if (ObjectsMap.Contains(ParticipantName))
		{
			// Update
			ObjectsMap[ParticipantName].Array.Add(Participant);
		}
		else
		{
			// Create
			FDlgObjectsArray ArrayStruct;
			ArrayStruct.Array.Add(Participant);
			ObjectsMap.Add(ParticipantName, ArrayStruct);
		}
	}

	return ObjectsMap;
}

TArray<UDlgDialogue*> UDlgManager::GetDialoguesWithDuplicateGUIDs()
{
	TArray<UDlgDialogue*> Dialogues = GetAllDialoguesFromMemory();
	TArray<UDlgDialogue*> DuplicateDialogues;

	TSet<FGuid> DialogueGUIDs;
	for (UDlgDialogue* Dialogue : Dialogues)
	{
		const FGuid ID = Dialogue->GetGUID();
		if (DialogueGUIDs.Find(ID) == nullptr)
		{
			// does not exist, good
			DialogueGUIDs.Add(ID);
		}
		else
		{
			// how?
			DuplicateDialogues.Add(Dialogue);
		}
	}

	return DuplicateDialogues;
}

TMap<FGuid, UDlgDialogue*> UDlgManager::GetAllDialoguesGUIDsMap()
{
	TArray<UDlgDialogue*> Dialogues = GetAllDialoguesFromMemory();
	TMap<FGuid, UDlgDialogue*> DialoguesMap;

	for (UDlgDialogue* Dialogue : Dialogues)
	{
		const FGuid ID = Dialogue->GetGUID();
		if (DialoguesMap.Contains(ID))
		{
			FDlgLogger::Get().Errorf(
				TEXT("GetAllDialoguesGUIDsMap - ID = `%s` for Dialogue = `%s` already exists"),
				*ID.ToString(), *Dialogue->GetPathName()
			);
		}

		DialoguesMap.Add(ID, Dialogue);
	}

	return DialoguesMap;
}

const TMap<FGuid, FDlgHistory>& UDlgManager::GetDialogueHistory()
{
	return FDlgMemory::Get().GetHistoryMaps();
}

void UDlgManager::SetDialogueHistory(const TMap<FGuid, FDlgHistory>& DlgHistory)
{
	FDlgMemory::Get().SetHistoryMap(DlgHistory);
}

void UDlgManager::ClearDialogueHistory()
{
	FDlgMemory::Get().Empty();
}

bool UDlgManager::DoesObjectImplementDialogueParticipantInterface(const UObject* Object)
{
	return FDlgHelper::IsObjectImplementingInterface(Object, UDlgDialogueParticipant::StaticClass());
}

bool UDlgManager::IsObjectACustomEvent(const UObject* Object)
{
	return FDlgHelper::IsObjectAChildOf(Object, UDlgEventCustom::StaticClass());
}

bool UDlgManager::IsObjectACustomCondition(const UObject* Object)
{
	return FDlgHelper::IsObjectAChildOf(Object, UDlgConditionCustom::StaticClass());
}

bool UDlgManager::IsObjectACustomTextArgument(const UObject* Object)
{
	return FDlgHelper::IsObjectAChildOf(Object, UDlgTextArgumentCustom::StaticClass());
}

bool UDlgManager::IsObjectANodeData(const UObject* Object)
{
	return FDlgHelper::IsObjectAChildOf(Object, UDlgNodeData::StaticClass());
}

TArray<UDlgDialogue*> UDlgManager::GetAllDialoguesForParticipantName(FName ParticipantName)
{
	TArray<UDlgDialogue*> DialoguesArray;
	for (UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		if (Dialogue->HasParticipant(ParticipantName))
		{
			DialoguesArray.Add(Dialogue);
		}
	}

	return DialoguesArray;
}

TArray<FName> UDlgManager::GetDialoguesParticipantNames()
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetParticipantNames());
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

TArray<FName> UDlgManager::GetDialoguesSpeakerStates()
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetSpeakerStates());
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

TArray<FName> UDlgManager::GetDialoguesParticipantIntNames(FName ParticipantName)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetParticipantIntNames(ParticipantName));
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

TArray<FName> UDlgManager::GetDialoguesParticipantFloatNames(FName ParticipantName)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetParticipantFloatNames(ParticipantName));
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

TArray<FName> UDlgManager::GetDialoguesParticipantBoolNames(FName ParticipantName)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetParticipantBoolNames(ParticipantName));
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

TArray<FName> UDlgManager::GetDialoguesParticipantFNameNames(FName ParticipantName)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetParticipantFNameNames(ParticipantName));
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

TArray<FName> UDlgManager::GetDialoguesParticipantConditionNames(FName ParticipantName)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetParticipantConditionNames(ParticipantName));
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

TArray<FName> UDlgManager::GetDialoguesParticipantEventNames(FName ParticipantName)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		UniqueNames.Append(Dialogue->GetParticipantEventNames(ParticipantName));
	}

	TArray<FName> Array;
	FDlgHelper::AppendSortedSetToArray(UniqueNames, Array);
	return Array;
}

bool UDlgManager::RegisterDialogueConsoleCommands()
{
	if (!IDlgSystemModule::IsAvailable())
	{
		FDlgLogger::Get().Error(TEXT("RegisterDialogueConsoleCommands - The Dialogue System Module is NOT Loaded"));
		return false;
	}

	IDlgSystemModule::Get().RegisterConsoleCommands(GetDialogueWorld());
	return true;
}

bool UDlgManager::UnregisterDialogueConsoleCommands()
{
	if (!IDlgSystemModule::IsAvailable())
	{
		FDlgLogger::Get().Error(TEXT("UnregisterDialogueConsoleCommands - The Dialogue System Module is NOT Loaded"));
		return false;
	}

	IDlgSystemModule::Get().UnregisterConsoleCommands();
	return true;
}

void UDlgManager::GatherParticipantsRecursive(UObject* Object, TArray<UObject*>& Array, TSet<UObject*>& AlreadyVisited)
{
	if (!IsValid(Object) || AlreadyVisited.Contains(Object))
	{
		return;
	}

	AlreadyVisited.Add(Object);
	if (Object->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
	{
		Array.Add(Object);
	}

	// Gather recursive from children
	for (auto* Property = Object->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		if (const auto* ObjectProperty = FNYReflectionHelper::CastProperty<FObjectProperty>(Property))
		{
			GatherParticipantsRecursive(ObjectProperty->GetPropertyValue_InContainer(Object), Array, AlreadyVisited);
		}

		// TODO: handle containers and structs
	}
}

UWorld* UDlgManager::GetDialogueWorld()
{
	// Try to use the user set one
	if (UserWorldContextObjectPtr.IsValid())
	{
		if (auto* WorldContextObject = UserWorldContextObjectPtr.Get())
		{
			if (auto* World = WorldContextObject->GetWorld())
			{
				return World;
			}
		}
	}

	// Fallback to default autodetection
	if (GEngine)
	{
		// Get first PIE world
		// Copied from TakeUtils::GetFirstPIEWorld()
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (!World || !World->IsPlayInEditor())
				continue;

			if (World->GetNetMode() == ENetMode::NM_Standalone ||
				(World->GetNetMode() == ENetMode::NM_Client && Context.PIEInstance == 2))
			{
				return World;
			}
		}

		// Otherwise get the first Game World
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (!World || !World->IsGameWorld())
				continue;

			return World;
		}
	}

	FDlgLogger::Get().Error(TEXT("GetDialogueWorld - Could NOT find any valid world. Call SetDialoguePersistentWorldContextObject in the Being Play of your GameMode"));
	return nullptr;
}
