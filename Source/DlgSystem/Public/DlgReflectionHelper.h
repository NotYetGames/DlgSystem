// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DlgSystemSettings.h"

#include "DlgReflectionHelper.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgSystemReflectionHelper, All, All)

UCLASS()
class DLGSYSTEM_API UDlgReflectionHelper : public UObject
{
	GENERATED_BODY()

public:

	UDlgReflectionHelper(const FObjectInitializer& ObjectInitializer) {};
	~UDlgReflectionHelper() {};

	/** Attempts to get the property VariableName from ParticipantObject*/
	template <typename PropertyType, typename VariableType>
	static VariableType GetVariable(const UObject* ParticipantObject, const FName VariableName);


	/** Attempts to modify the property VariableName from ParticipantObject */
	template <typename PropertyType, typename VariableType>
	static void ModifyVariable(UObject* ParticipantObject, const FName VariableName, const VariableType Value, bool bDelta);


	/** Attempts to set the property VariableName from ParticipantObject */
	template <typename PropertyType, typename VariableType>
	static void SetVariable(UObject* ParticipantObject, const FName VariableName, const VariableType NewValue);


	/**
	 * Gathers the name of all variables in ParticipantClass (except those properties that belong to the black listed classes) from the type defined by PropertyClass
	 * If the BlacklistedClasses is empty it will get the default ones from the settings
	 */
	template <typename ContainerType>
	static void GetVariableNames(const UClass* ParticipantClass, const UClass* PropertyClass,
								 ContainerType& OutContainer, TArray<UClass*> BlacklistedClasses = {});
};


template <typename PropertyType, typename VariableType>
VariableType UDlgReflectionHelper::GetVariable(const UObject* ParticipantObject, const FName VariableName)
{
	if (!IsValid(ParticipantObject))
	{
		UE_LOG(LogDlgSystemReflectionHelper,
			   Error,
		       TEXT("Failed to get %s %s from ParticipantObject that is null (not valid)"),
		      *PropertyType::StaticClass()->GetName(), *VariableName.ToString());
		return VariableType{};
	}

	for (UProperty* Property = ParticipantObject->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		const PropertyType* CastedProperty = Cast<PropertyType>(Property);
		if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
		{
			return CastedProperty->GetPropertyValue_InContainer(ParticipantObject, 0);
		}
	}

	UE_LOG(LogDlgSystemReflectionHelper,
		   Warning,
		   TEXT("Failed to get %s %s from %s - property not found!"),
		   *PropertyType::StaticClass()->GetName(), *VariableName.ToString(), *ParticipantObject->GetName());
	return VariableType{};
}


template <typename PropertyType, typename VariableType>
void UDlgReflectionHelper::ModifyVariable(UObject* ParticipantObject, const FName VariableName, const VariableType Value, bool bDelta)
{
	if (!IsValid(ParticipantObject))
	{
		UE_LOG(LogDlgSystemReflectionHelper,
			   Error,
		       TEXT("Failed to modify %s %s of ParticipantObject that is null (not valid)"),
		      *PropertyType::StaticClass()->GetName(), *VariableName.ToString());
		return;
	}

	// Set the variable
	if (!bDelta)
	{
		SetVariable<PropertyType>(ParticipantObject, VariableName, Value);
		return;
	}

	// Modify the current variable
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

	UE_LOG(LogDlgSystemReflectionHelper,
		   Warning,
		   TEXT("Failed to modify %s %s from %s - property not found!"),
		   *PropertyType::StaticClass()->GetName(), *VariableName.ToString(), *ParticipantObject->GetName());
}


template <typename PropertyType, typename VariableType>
void UDlgReflectionHelper::SetVariable(UObject* ParticipantObject, const FName VariableName, const VariableType NewValue)
{
	if (!IsValid(ParticipantObject))
	{
		UE_LOG(LogDlgSystemReflectionHelper,
			   Error,
		       TEXT("Failed to set %s %s of ParticipantObject that is null (not valid)"),
		      *PropertyType::StaticClass()->GetName(), *VariableName.ToString());
		return;
	}

	for (UProperty* Property = ParticipantObject->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
	{
		const PropertyType* CastedProperty = Cast<PropertyType>(Property);
		if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
		{
			CastedProperty->SetPropertyValue_InContainer(ParticipantObject, NewValue);
			return;
		}
	}

	UE_LOG(LogDlgSystemReflectionHelper,
		   Warning,
		   TEXT("Failed to set %s %s from %s - property not found!"),
		   *PropertyType::StaticClass()->GetName(), *VariableName.ToString(), *ParticipantObject->GetName());
}


template <typename ContainerType>
void UDlgReflectionHelper::GetVariableNames(const UClass* ParticipantClass, const UClass* PropertyClass,
	ContainerType& OutContainer, TArray<UClass*> BlacklistedClasses)
{
	if (!IsValid(ParticipantClass) || !IsValid(PropertyClass))
	{
		return;
	}

	// Use the system settings
	if (BlacklistedClasses.Num() == 0)
	{
		BlacklistedClasses = GetDefault<UDlgSystemSettings>()->BlacklistedReflectionClasses;
	}

	auto IsPropertyAtStartOfBlacklistedClass = [&BlacklistedClasses](UProperty* CheckProperty) -> bool
	{
		for (UClass* Class : BlacklistedClasses)
		{
			if (!IsValid(Class))
			{
				continue;
			}

			// CheckProperty is at the start of the blacklisted class
			if (Class->PropertyLink == CheckProperty)
			{
				return true;
			}
		}

		return false;
	};

	// Property link goes from the left to right where on the left there are the most inner child properties and at the right ther are the top most parents.
	UProperty* Property = ParticipantClass->PropertyLink;
	while (Property != nullptr && !IsPropertyAtStartOfBlacklistedClass(Property))
	{
		if (Property->GetClass() == PropertyClass)
		{
			OutContainer.Add(Property->GetFName());
		}
		Property = Property->PropertyLinkNext;
	}
}
