// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgManager.h"

#include "UObject/UObjectIterator.h"
#include "Engine/ObjectLibrary.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/Blueprint.h"

#include "IDlgSystemModule.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgDialogueParticipant.h"
#include "DlgDialogue.h"
#include "DlgMemory.h"
#include "DlgContextInternal.h"

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
	TArray<UObject*> Participants;
	Participants.Add(Participant);
	return StartDialogue(Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue2(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1)
{
	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	return StartDialogue(Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue3(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2)
{
	TArray<UObject*> Participants;
	Participants.Add(Participant0);
	Participants.Add(Participant1);
	Participants.Add(Participant2);
	return StartDialogue(Dialogue, Participants);
}

UDlgContext* UDlgManager::StartDialogue4(UDlgDialogue* Dialogue, UObject* Participant0, UObject* Participant1, UObject* Participant2, UObject* Participant3)
{
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
	TArray<FString> PathsToSeach = { TEXT("/Game") };
	ObjectLibrary->AddToRoot();

	// Add the current plugin dir
	// TODO maybe add all the non engine plugin paths? IPluginManager::Get().GetEnabledPlugins()
	TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(DIALOGUE_SYSTEM_PLUGIN_NAME.ToString());
	if (ThisPlugin.IsValid())
	{
		FString PluginPath = ThisPlugin->GetMountedAssetPath();
		// See NOTE above
		PluginPath.RemoveFromEnd(TEXT("/"));
		PathsToSeach.Add(PluginPath);
	}

	ObjectLibrary->LoadAssetDataFromPaths(PathsToSeach);
	const int32 Count = ObjectLibrary->LoadAssetsFromAssetData();
	ObjectLibrary->RemoveFromRoot();
	return Count;
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
			UE_LOG(LogDlgSystem,
				Error,
				TEXT("GetAllDialoguesGuidMap: ID = `%s` for Dialogue = `%s` already exists"), *ID.ToString(), *Dialogue->GetPathName());
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

bool UDlgManager::ConstructParticipantMap(const UDlgDialogue* Dialogue, const TArray<UObject*>& Participants, TMap<FName, UObject*>& OutMap)
{
	if (!IsValid(Dialogue) || Dialogue->GetParticipantData().Num() == 0)
	{
		UE_LOG(LogDlgSystem,
			   Error,
			   TEXT("Failed to start dialogue - Invalid dialogue! (Either nullptr or a dialogue without any participants)"));
		return false;
	}

	// check participant data:
	const TMap<FName, FDlgParticipantData>& DialogueParticipants = Dialogue->GetParticipantData();
	if (DialogueParticipants.Num() != Participants.Num())
	{
		UE_LOG(LogDlgSystem,
			   Error,
			   TEXT("Failed to start dialogue - the amount of participants does not match the dialogue's "
					"expectation! Dialogue Participants Num %d != Participants Num %d"),
			   DialogueParticipants.Num(), Participants.Num());
		return false;
	}

	const int32 ParticipantsNum = Participants.Num();
	for (int32 ParticipantIndex = 0; ParticipantIndex < ParticipantsNum; ParticipantIndex++)
	{
		UObject* Participant = Participants[ParticipantIndex];
		if (!IsValid(Participant))
		{
			UE_LOG(LogDlgSystem, Error, TEXT("Failed to start dialogue - Participant at index %d is null"), ParticipantIndex);
			return false;
		}

		// Be sure it is a participant
		if (!Participant->GetClass()->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
		{
			UE_LOG(LogDlgSystem,
				   Error,
				   TEXT("Failed to start dialogue - Participant object at index = %d with ObjectName = `%s`"
						"does not implement the IDlgDialogueParticipant interface!"),
				   ParticipantIndex, *Participant->GetName());
			return false;
		}

		// Does the participant name exist in the Dialogue?
		const FName ParticipantName = IDlgDialogueParticipant::Execute_GetParticipantName(Participant);
		if (!DialogueParticipants.Find(ParticipantName))
		{
			UE_LOG(LogDlgSystem,
				   Error,
				   TEXT("Failed to start dialogue - Input Participant at index = %d "
						"does not have a participant with name = `%s` in the Dialogue"),
				   ParticipantIndex, *ParticipantName.ToString());
			return false;
		}
		OutMap.Add(ParticipantName, Participant);
	}

	return true;
}
