// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "GameFramework/Character.h"
#include "DlgReflectionHelper.generated.h"


UCLASS()
class DLGSYSTEM_API UDlgReflectionHelper : public UObject
{
	GENERATED_BODY()

public:

	UDlgReflectionHelper(const FObjectInitializer& ObjectInitializer) {};
	~UDlgReflectionHelper() {};

	/** Attempts to get the property VariableName from ParticipantObject*/
	template <typename PropertyType, typename VariableType>
	static VariableType GetVariable(UObject* ParticipantObject, FName VariableName);


	/** Attempts to modify the property VariableName from ParticipantObject */
	template <typename PropertyType, typename VariableType>
	static void ModifyVariable(UObject* ParticipantObject, FName VariableName, VariableType Value, bool bDelta);


	/** Attempts to set the property VariableName from ParticipantObject */
	template <typename PropertyType, typename VariableType>
	static void SetVariable(UObject* ParticipantObject, FName VariableName, VariableType NewValue);


	/** Gathers the name of all variables in ParticipantClass from the type defined by PropertyClass */
	template <typename ContainerType>
	static void GetVariableNames(UClass* ParticipantClass, UClass* PropertyClass, ContainerType& OutContainer);
};


template <typename PropertyType, typename VariableType>
VariableType UDlgReflectionHelper::GetVariable(UObject* ParticipantObject, FName VariableName)
{
	for (UProperty* Property = ParticipantObject->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		const PropertyType* CastedProperty = Cast<PropertyType>(Property);
		if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
		{
			return CastedProperty->GetPropertyValue_InContainer(ParticipantObject, 0);
		}
	}

	UE_LOG(LogDlgSystem, Warning, TEXT("Failed to get %s %s from %s - property not found!"), 
									   *PropertyType::StaticClass()->GetName(), 
									   *VariableName.ToString(), 
									   *ParticipantObject->GetName());
	return VariableType{};
}


template <typename PropertyType, typename VariableType>
void UDlgReflectionHelper::ModifyVariable(UObject* ParticipantObject, FName VariableName, VariableType Value, bool bDelta)
{
	if (!bDelta)
	{
		SetVariable<PropertyType>(ParticipantObject, VariableName, Value);
		return;
	}

	for (UProperty* Property = ParticipantObject->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		const PropertyType* CastedProperty = Cast<PropertyType>(Property);
		if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
		{
			const VariableType OldValue = CastedProperty->GetPropertyValue_InContainer(ParticipantObject, 0);
			CastedProperty->SetPropertyValue_InContainer(ParticipantObject, OldValue + Value);
			return;
		}
	}

	UE_LOG(LogDlgSystem, Warning, TEXT("Failed to modify %s %s from %s - property not found!"), 
									   *PropertyType::StaticClass()->GetName(), 
									   *VariableName.ToString(), 
									   *ParticipantObject->GetName());

}


template <typename PropertyType, typename VariableType>
void UDlgReflectionHelper::SetVariable(UObject* ParticipantObject, FName VariableName, VariableType NewValue)
{
	for (UProperty* Property = ParticipantObject->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		const PropertyType* CastedProperty = Cast<PropertyType>(Property);
		if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
		{
			CastedProperty->SetPropertyValue_InContainer(ParticipantObject, NewValue);
			return;
		}
	}

	UE_LOG(LogDlgSystem, Warning, TEXT("Failed to set %s %s from %s - property not found!"), 
									   *PropertyType::StaticClass()->GetName(), 
									   *VariableName.ToString(), 
									   *ParticipantObject->GetName());
}


template <typename ContainerType>
void UDlgReflectionHelper::GetVariableNames(UClass* ParticipantClass, UClass* PropertyClass, ContainerType& OutContainer)
{
	if (ParticipantClass == nullptr)
	{
		return;
	}

	UProperty* Property = ParticipantClass->PropertyLink;

	UProperty* ActorPropertyBegin = AActor::StaticClass()->PropertyLink;
	UProperty* PawnPropertyBegin = APawn::StaticClass()->PropertyLink;
	UProperty* CharacterPropertyBegin = ACharacter::StaticClass()->PropertyLink;

	while (Property != nullptr && Property != ActorPropertyBegin && Property != PawnPropertyBegin && Property != CharacterPropertyBegin)
	{
		if (Property->GetClass() == PropertyClass)
		{
			OutContainer.Add(Property->GetFName());
		}
		Property = Property->PropertyLinkNext;
	}
}
