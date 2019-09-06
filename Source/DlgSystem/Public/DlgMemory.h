// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DlgMemory.generated.h"

/**
 *  Data per dialogue asset
 */
USTRUCT(Blueprintable, BlueprintType)
struct DLGSYSTEM_API FDlgHistory
{
	GENERATED_USTRUCT_BODY()
public:
	/** list of already visited node indices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueHistoryData)
	TSet<int32> VisitedNodeIndices;

public:
	bool operator==(const FDlgHistory& Other) const;
};

/**
 *  Singleton to store Dialogue history
 */
class DLGSYSTEM_API FDlgMemory
{
public:
	static FDlgMemory* GetInstance()
	{
		static FDlgMemory Instance;
		return &Instance;
	}

	/** removes all entries */
	void Empty() { HistoryMap.Empty(); }

	/** adds an entry to the map or overrides an existing one */
	void SetEntry(const FGuid& DlgGuid, const FDlgHistory& History);

	/** returns the entry for the given name, or nullptr if it does not exist */
	FDlgHistory* GetEntry(const FGuid& DlgGuid) { return HistoryMap.Find(DlgGuid); }

	void SetNodeVisited(const FGuid& DlgGuid, int32 NodeIndex);
	bool IsNodeVisited(const FGuid& DlgGuid, int32 NodeIndex) const;

	const TMap<FGuid, FDlgHistory>& GetHistoryMaps() const { return HistoryMap; }
	void SetHistoryMap(const TMap<FGuid, FDlgHistory>& Map) { HistoryMap = Map; }

	void Serialize(FArchive& Ar);
private:

	/**
	 *  Key: Dialogue unique identifier Guid
	 *  Value: set of already visited nodes
	 */
	UPROPERTY()
	TMap<FGuid, FDlgHistory> HistoryMap;
};

// operator overloads for serialization
FORCEINLINE FArchive& operator<<(FArchive &Ar, FDlgHistory& History)
{
	Ar << History.VisitedNodeIndices;
	return Ar;
}
