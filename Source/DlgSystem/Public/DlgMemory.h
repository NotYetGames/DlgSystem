// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "DlgMemory.generated.h"

USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgHistory
{
	GENERATED_USTRUCT_BODY()
public:
	// List of already visited node indices
	// NOTE: if you serialize this but then later change the dialogue node positions this will have the wrong indices
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|History")
	TSet<int32> VisitedNodeIndices;

public:
	bool operator==(const FDlgHistory& Other) const;
};

// Singleton to store Dialogue history
// TODO: investigate if this is multiplayer friendly, it does not seem so as there exists only a single global dialogue memory
USTRUCT()
struct DLGSYSTEM_API FDlgMemory
{
	GENERATED_USTRUCT_BODY()
public:
	static FDlgMemory* GetInstance()
	{
		static FDlgMemory Instance;
		return &Instance;
	}
	static FDlgMemory& Get()
	{
		auto* Instance = GetInstance();
		check(Instance != nullptr);
		return *Instance;
	}

	// Removes all entries
	void Empty() { HistoryMap.Empty(); }

	// Adds an entry to the map or overrides an existing one
	void SetEntry(const FGuid& DialogueGUID, const FDlgHistory& History);

	// Returns the entry for the given name, or nullptr if it does not exist */
	FDlgHistory* GetEntry(const FGuid& DialogueGUID) { return HistoryMap.Find(DialogueGUID); }

	void SetNodeVisited(const FGuid& DialogueGUID, int32 NodeIndex);
	bool IsNodeVisited(const FGuid& DialogueGUID, int32 NodeIndex) const;

	const TMap<FGuid, FDlgHistory>& GetHistoryMaps() const { return HistoryMap; }
	void SetHistoryMap(const TMap<FGuid, FDlgHistory>& Map) { HistoryMap = Map; }

	void Serialize(FArchive& Ar);

private:
	 // Key: Dialogue unique identifier Guid
	 // Value: set of already visited nodes
	UPROPERTY()
	TMap<FGuid, FDlgHistory> HistoryMap;
};

template<>
struct TStructOpsTypeTraits<FDlgHistory> : public TStructOpsTypeTraitsBase2<FDlgHistory>
{
	enum
	{
		WithIdenticalViaEquality = true
    };
};
