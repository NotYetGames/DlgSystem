// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"

#include "DlgParticipantName.generated.h"


// Helper struct to provide a way to reference "Participants" in custom events and conditions
// Custom UI picker is supported to set it, and the DlgContext has a helper function to get the participant from the name
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgParticipantName
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName ParticipantName;
};
