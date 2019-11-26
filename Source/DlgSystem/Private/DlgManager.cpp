// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgManager.h"

#include "UObject/UObjectIterator.h"
#include "Engine/ObjectLibrary.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/Blueprint.h"
#include "EngineUtils.h"

#include "IDlgSystemModule.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgDialogueParticipant.h"
#include "DlgDialogue.h"
#include "DlgMemory.h"
#include "DlgContextInternal.h"
#include "Logging/DlgLogger.h"
#include "DlgHelper.h"

UDlgContext* UDlgManager::StartDialogue(UDlgDialogue* Dialogue, const TArray<UObject*>& Participants)
{
	TMap<FName, UObject*> ParticipantBinding;

	if (!ConstructParticipantMap(Dialogue, Participants, ParticipantBinding))
	{
		return nullptr;
	}

	UDlgContextInternal* Context = NewObject<UDlgContextInternal>(Participants[0], UDlgContextInternal::StaticClass());
	if (Context->Initialize(Dialogue, ParticipantBinding))
	{
		return Context;
	}

	return nullptr;
}


bool UDlgManager::CouldStartDialogue(UDlgDialogue* Dialogue, UPARAM(ref)const TArray<UObject*>& Participants)
{
	TMap<FName, UObject*> ParticipantBinding;

	if (!ConstructParticipantMap(Dialogue, Participants, ParticipantBinding))
	{
		return false;
	}

	UDlgContextInternal* Context = NewObject<UDlgContextInternal>(Participants[0], UDlgContextInternal::StaticClass());
	return Context->CouldBeInitialized(Dialogue, ParticipantBinding);
}


UDlgContext* UDlgManager::ResumeDialogue(UDlgDialogue* Dialogue, UPARAM(ref)const TArray<UObject*>& Participants, int32 StartIndex, const TSet<int32>& AlreadyVisitedNodes, bool bFireEnterEvents)
{
	TMap<FName, UObject*> ParticipantBinding;

	if (!ConstructParticipantMap(Dialogue, Participants, ParticipantBinding))
	{
		return nullptr;
	}

	UDlgContextInternal* Context = NewObject<UDlgContextInternal>(Participants[0], UDlgContextInternal::StaticClass());
	if (Context->Initialize(Dialogue, ParticipantBinding, StartIndex, AlreadyVisitedNodes, bFireEnterEvents))
	{
		return Context;
	}

	return nullptr;
}


UDlgContext* UDlgManager::StartMonologue(UDlgDialogue* Dialogue, UObject* Participant)
{
	if (!ValidateParticipant(TEXT("StartMonologue argument = `Participant`"), Dialogue, Participant))
		return nullptr;

	TArray<UObject*> Participants;
	Participants.Add(Participant);
	return StartDialogue(Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue2(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1)
{
	if (!ValidateParticipant(TEXT("StartDialogue2 argument = `Participant0`"), Dialogue, Participant0))
		return nullptr;
	if (!ValidateParticipant(TEXT("StartDialogue2 argument = `Participant1`"), Dialogue, Participant1))
		return nullptr;

	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	return StartDialogue(Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue3(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2)
{
	if (!ValidateParticipant(TEXT("StartDialogue3 argument = `Participant0`"), Dialogue, Participant0))
		return nullptr;
	if (!ValidateParticipant(TEXT("StartDialogue3 argument = `Participant1`"), Dialogue, Participant1))
		return nullptr;
	if (!ValidateParticipant(TEXT("StartDialogue3 argument = `Participant2`"), Dialogue, Participant2))
		return nullptr;

	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	Participants.Add(Participant2);
	return StartDialogue(Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue4(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2, UObject* Participant3)
{
	if (!ValidateParticipant(TEXT("StartDialogue4 argument = `Participant0`"), Dialogue, Participant0))
		return nullptr;
	if (!ValidateParticipant(TEXT("StartDialogue4 argument = `Participant1`"), Dialogue, Participant1))
		return nullptr;
	if (!ValidateParticipant(TEXT("StartDialogue4 argument = `Participant2`"), Dialogue, Participant2))
		return nullptr;
	if (!ValidateParticipant(TEXT("StartDialogue4 argument = `Participant3`"), Dialogue, Participant3))
		return nullptr;

	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	Participants.Add(Participant2);
	Participants.Add(Participant3);
	return StartDialogue(Dialogue, Participants);
}

int32 UDlgManager::LoadAllDialoguesIntoMemory()
{
	// NOTE: All paths must NOT have the forward slash "/" at the end.
	// If they do, then this won't load Dialogues that are located in the Content root directory
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(UDlgDialogue::StaticClass(), true, GIsEditor);
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

	ObjectLibrary->LoadAssetDataFromPaths(PathsToSearch);
	const int32 Count = ObjectLibrary->LoadAssetsFromAssetData();
	ObjectLibrary->RemoveFromRoot();
	return Count;
}

TArray<UDlgDialogue*> UDlgManager::GetAllDialoguesFromMemory()
{
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

TArray<TWeakObjectPtr<AActor>> UDlgManager::GetAllActorsImplementingDialogueParticipantInterface(UWorld* World)
{
	TArray<TWeakObjectPtr<AActor>> Array;
	for (TActorIterator<AActor> Itr(World); Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		if (IsValid(Actor) && Actor->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
		{
			Array.Add(Actor);
		}
	}
	return Array;
}

TArray<UDlgDialogue*> UDlgManager::GetDialoguesWithDuplicateGuid()
{
	TArray<UDlgDialogue*> Dialogues = GetAllDialoguesFromMemory();
	TArray<UDlgDialogue*> DuplicateDialogues;

	TSet<FGuid> DialogueGuids;
	for (UDlgDialogue* Dialogue : Dialogues)
	{
		const FGuid id = Dialogue->GetDlgGuid();
		if (DialogueGuids.Find(id) == nullptr)
		{
			// does not exist, good
			DialogueGuids.Add(id);
		}
		else
		{
			// how?
			DuplicateDialogues.Add(Dialogue);
		}
	}

	return DuplicateDialogues;
}

TMap<FGuid, UDlgDialogue*> UDlgManager::GetAllDialoguesGuidMap()
{
	TArray<UDlgDialogue*> Dialogues = GetAllDialoguesFromMemory();
	TMap<FGuid, UDlgDialogue*> DialoguesMap;

	for (UDlgDialogue* Dialogue : Dialogues)
	{
		const FGuid ID = Dialogue->GetDlgGuid();
		if (DialoguesMap.Contains(ID))
		{
			FDlgLogger::Get().Errorf(
				TEXT("GetAllDialoguesGuidMap: ID = `%s` for Dialogue = `%s` already exists"),
				*ID.ToString(), *Dialogue->GetPathName()
			);
		}

		DialoguesMap.Add(ID, Dialogue);
	}

	return DialoguesMap;
}

const TMap<FGuid, FDlgHistory>& UDlgManager::GetDialogueHistory()
{
	return FDlgMemory::GetInstance()->GetHistoryMaps();
}

void UDlgManager::SetDialogueHistory(const TMap<FGuid, FDlgHistory>& DlgHistory)
{
	FDlgMemory::GetInstance()->SetHistoryMap(DlgHistory);
}

void UDlgManager::ClearDialogueHistory()
{
	FDlgMemory::GetInstance()->Empty();
}

bool UDlgManager::DoesObjectImplementDialogueParticipantInterface(const UObject* Object)
{
	static const UClass* DialogueParticipantClass = UDlgDialogueParticipant::StaticClass();

	// Apparently blueprints only work this way
	// NOTE this is the blueprint assets, not an instance, used only by the custom graph nodes
	if (const UBlueprint* Blueprint = Cast<UBlueprint>(Object))
	{
		if (const UClass* GeneratedClass = Cast<UClass>(Blueprint->GeneratedClass))
		{
			return GeneratedClass->ImplementsInterface(DialogueParticipantClass);
		}
	}

	// A class object, does this ever happen?
	if (const UClass* Class = Cast<UClass>(Object))
	{
		return Class->ImplementsInterface(DialogueParticipantClass);
	}

	// All other object types
	return Object->GetClass()->ImplementsInterface(DialogueParticipantClass);
}

TArray<UDlgDialogue*> UDlgManager::GetAllDialoguesForParticipantName(const FName& ParticipantName)
{
	TArray<UDlgDialogue*> DialoguesArray;
	for (UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		if (Dialogue->IsParticipant(ParticipantName))
		{
			DialoguesArray.Add(Dialogue);
		}
	}

	return DialoguesArray;
}

void UDlgManager::GetAllDialoguesParticipantNames(TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetAllParticipantNames(UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

void UDlgManager::GetAllDialoguesSpeakerStates(TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetAllSpeakerState(UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

void UDlgManager::GetAllDialoguesIntNames(const FName& ParticipantName, TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetIntNames(ParticipantName, UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

void UDlgManager::GetAllDialoguesFloatNames(const FName& ParticipantName, TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetFloatNames(ParticipantName, UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

void UDlgManager::GetAllDialoguesBoolNames(const FName& ParticipantName, TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetBoolNames(ParticipantName, UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

void UDlgManager::GetAllDialoguesNameNames(const FName& ParticipantName, TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetNameNames(ParticipantName, UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

void UDlgManager::GetAllDialoguesConditionNames(const FName& ParticipantName, TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetConditions(ParticipantName, UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

void UDlgManager::GetAllDialoguesEventNames(const FName& ParticipantName, TArray<FName>& OutArray)
{
	TSet<FName> UniqueNames;
	for (const UDlgDialogue* Dialogue : GetAllDialoguesFromMemory())
	{
		Dialogue->GetEvents(ParticipantName, UniqueNames);
	}

	FDlgHelper::AppendSortedSetToArray(UniqueNames, OutArray);
}

bool UDlgManager::RegisterDialogueModuleConsoleCommands(AActor* InReferenceActor)
{
	if (!IDlgSystemModule::IsAvailable())
	{
		return false;
	}

	IDlgSystemModule::Get().RegisterConsoleCommands(InReferenceActor);
	return true;
}

bool UDlgManager::UnRegisterDialogueModuleConsoleCommands()
{
	if (!IDlgSystemModule::IsAvailable())
	{
		return false;
	}

	IDlgSystemModule::Get().UnregisterConsoleCommands();
	return true;
}

bool UDlgManager::ValidateParticipant(const FString& ContextMessageFailure, const UDlgDialogue* ContextDialogue, UObject* Participant)
{
	const FString DialoguePath = IsValid(ContextDialogue) ? ContextDialogue->GetPathName() : TEXT("NONE");
	if (!IsValid(Participant))
	{
		FDlgLogger::Get().Errorf(
			TEXT("%s - Participant is invalid (not set or nullptr). For Dialogue = `%s`"),
			*ContextMessageFailure, *DialoguePath
		);
		return false;
	}
	if (!Participant->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
	{
		FDlgLogger::Get().Errorf(
			TEXT("%s - Participant = `%s` does not implement the IDlgDialogueParticipant interface!. For Dialogue = `%s`"),
			*ContextMessageFailure, *Participant->GetPathName(), *DialoguePath
		);
		return false;
	}

	return true;
}

bool UDlgManager::ConstructParticipantMap(const UDlgDialogue* Dialogue, const TArray<UObject*>& Participants, TMap<FName, UObject*>& OutMap)
{
	if (!IsValid(Dialogue))
	{
		FDlgLogger::Get().Error(TEXT("Failed to start dialogue - Invalid dialogue (is nullptr)!"));
		return false;
	}

	if (Dialogue->GetParticipantData().Num() == 0)
	{
		FDlgLogger::Get().Errorf(TEXT("Failed to start dialogue = `%s` - Dialogue does not have any participants"), *Dialogue->GetPathName());
		return false;
	}

	// check participant data:
	const TMap<FName, FDlgParticipantData>& DialogueParticipants = Dialogue->GetParticipantData();
	if (DialogueParticipants.Num() != Participants.Num())
	{
		FDlgLogger::Get().Errorf(
			TEXT("Failed to start dialogue = `%s` - The asset has %d participants! Provided participant count: %d"),
			*Dialogue->GetPathName(), DialogueParticipants.Num(), Participants.Num()
		);
		return false;
	}

	const int32 ParticipantsNum = Participants.Num();
	for (int32 ParticipantIndex = 0; ParticipantIndex < ParticipantsNum; ParticipantIndex++)
	{
		UObject* Participant = Participants[ParticipantIndex];
		if (!IsValid(Participant))
		{
			FDlgLogger::Get().Errorf(
				TEXT("Failed to start dialogue = `%s` - Participant at index %d is invalid (not set or nullptr)"),
				*Dialogue->GetPathName(), ParticipantIndex
			);
			return false;
		}

		// Be sure it is a participant
		if (!Participant->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
		{
			FDlgLogger::Get().Errorf(
				TEXT("Failed to start dialogue = `%s` - Participant object at index = %d and Path = `%s` does not implement the IDlgDialogueParticipant interface!"),
				*Dialogue->GetPathName(), ParticipantIndex, *Participant->GetPathName()
			);
			return false;
		}

		// Does the participant name exist in the Dialogue?
		const FName ParticipantName = IDlgDialogueParticipant::Execute_GetParticipantName(Participant);
		if (!DialogueParticipants.Find(ParticipantName))
		{
			FDlgLogger::Get().Errorf(
				TEXT("Failed to start dialogue = `%s` - Input Participant at index = %d has the name %s, which is not referenced by this Dialogue"),
				*Dialogue->GetPathName(), ParticipantIndex, *ParticipantName.ToString()
			);
			return false;
		}
		OutMap.Add(ParticipantName, Participant);
	}

	return true;
}
