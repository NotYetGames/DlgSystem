// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

// Protect against inclusion in multiple projects
#ifndef NY_REFLECTION_HELPER
#define NY_REFLECTION_HELPER

#include "CoreMinimal.h"
#include "NYReflectionTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogDlgSystemReflectionHelper, All, All)

class DLGSYSTEM_API FNYReflectionHelper
{
public:

#if ENGINE_MINOR_VERSION >= 25
	// From 4.25 UProperties are different
	template<typename FieldType>
	FORCEINLINE static FieldType* CastProperty(FField* Src)
	{
		return Src && Src->HasAnyCastFlags(FieldType::StaticClassCastFlagsPrivate()) ? static_cast<FieldType*>(Src) : nullptr;
	}
	// Const Version
	template<typename FieldType>
	FORCEINLINE static const FieldType* CastProperty(const FField* Src)
	{
		return Src && Src->HasAnyCastFlags(FieldType::StaticClassCastFlagsPrivate()) ? static_cast<const FieldType*>(Src) : nullptr;
	}

	FORCEINLINE static FField* GetStructChildren(const UStruct* Struct)
	{
		return Struct ? Struct->ChildProperties : nullptr;
	}

	// the old removed engine function this code still uses a lot:
	template <typename To, typename From>
	static To* SmartCastProperty(From* Src)
	{
		To* Result = CastProperty<To>(Src);
		if (Result == nullptr)
		{
			FNYArrayProperty* ArrayProp = CastProperty<FNYArrayProperty>(Src);
			if (ArrayProp != nullptr)
			{
				Result = CastProperty<To>(ArrayProp->Inner);
			}
		}
		return Result;
	}

#else
	template <typename To, typename From>
	FORCEINLINE static To* CastProperty(From* Src)
	{
		return TCastImpl<From, To>::DoCast(Src);
	}
	// Const Version
	template <typename To, typename From>
	FORCEINLINE static const To* CastProperty(const From* Src)
	{
		return CastProperty<To>(const_cast<From*>(Src));
	}

	FORCEINLINE static UField* GetStructChildren(const UStruct* Struct)
	{
		return Struct ? Struct->Children : nullptr;
	}

	// the old removed engine function this code still uses a lot:
	template <typename To, typename From>
	static To* SmartCastProperty(From* Src)
	{
		To* Result = dynamic_cast<To*>(Src);
		if (Result == nullptr)
		{
			FNYArrayProperty* ArrayProp = dynamic_cast<FNYArrayProperty*>(Src);
			if (ArrayProp != nullptr)
			{
				Result = dynamic_cast<To*>(ArrayProp->Inner);
			}
		}
		return Result;
	}
#endif // ENGINE_MINOR_VERSION >= 25

	// Attempts to get the property VariableName from Object
	template <typename PropertyType, typename VariableType>
	static VariableType GetVariable(const UObject* Object, FName VariableName)
	{
		if (!IsValid(Object))
		{
			UE_LOG(
				LogDlgSystemReflectionHelper,
				Error,
				TEXT("Failed to get %s %s from Object that is null (not valid)"),
				*PropertyType::StaticClass()->GetName(), *VariableName.ToString()
			);
			return VariableType{};
		}

		for (auto* Property = Object->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			const PropertyType* CastedProperty = CastProperty<PropertyType>(Property);
			if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
			{
				return CastedProperty->GetPropertyValue_InContainer(Object, 0);
			}
		}

		UE_LOG(
			LogDlgSystemReflectionHelper,
			Warning,
			TEXT("Failed to get %s %s from %s - property not found!"),
			*PropertyType::StaticClass()->GetName(), *VariableName.ToString(), *Object->GetName()
		);
		return VariableType{};
	}

	// Attempts to modify the property VariableName from Object
	template <typename PropertyType, typename VariableType>
	static void ModifyVariable(UObject* Object, FName VariableName, const VariableType Value, bool bDelta)
	{
		if (!IsValid(Object))
		{
			UE_LOG(
				LogDlgSystemReflectionHelper,
				Error,
				TEXT("Failed to modify %s %s of Object that is null (not valid)"),
				*PropertyType::StaticClass()->GetName(), *VariableName.ToString()
			);
			return;
		}

		// Set the variable
		if (!bDelta)
		{
			SetVariable<PropertyType>(Object, VariableName, Value);
			return;
		}

		// Modify the current variable
		for (auto* Property = Object->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			const PropertyType* CastedProperty = CastProperty<PropertyType>(Property);
			if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
			{
				const VariableType OldValue = CastedProperty->GetPropertyValue_InContainer(Object, 0);
				CastedProperty->SetPropertyValue_InContainer(Object, OldValue + Value);
				return;
			}
		}

		UE_LOG(
			LogDlgSystemReflectionHelper,
			Warning,
			TEXT("Failed to modify %s %s from %s - property not found!"),
			*PropertyType::StaticClass()->GetName(), *VariableName.ToString(), *Object->GetName()
		);
	}


	// Attempts to set the property VariableName from Object
	template <typename PropertyType, typename VariableType>
	static void SetVariable(UObject* Object, FName VariableName, const VariableType NewValue)
	{
		if (!IsValid(Object))
		{
			UE_LOG(
				LogDlgSystemReflectionHelper,
				Error,
				TEXT("Failed to set %s %s of Object that is null (not valid)"),
				*PropertyType::StaticClass()->GetName(), *VariableName.ToString()
			);
			return;
		}

		for (auto* Property = Object->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			const PropertyType* CastedProperty = CastProperty<PropertyType>(Property);
			if (CastedProperty != nullptr && CastedProperty->GetFName() == VariableName)
			{
				CastedProperty->SetPropertyValue_InContainer(Object, NewValue);
				return;
			}
		}

		UE_LOG(
			LogDlgSystemReflectionHelper,
			Warning,
			TEXT("Failed to set %s %s from %s - property not found!"),
			*PropertyType::StaticClass()->GetName(), *VariableName.ToString(), *Object->GetName()
		);
	}

	// Gathers the name of all variables in ParticipantClass (except those properties that belong to the black listed classes) from the type defined by PropertyClass
	// If the BlacklistedClasses is empty it will get the default ones from the settings
	template <typename ContainerType>
	static void GetVariableNames(
		const UClass* ParticipantClass,
		const FNYPropertyClass* PropertyClass,
		ContainerType& OutContainer,
		const TArray<UClass*>& BlacklistedClasses
	)
	{
		if (!IsValid(ParticipantClass) || !PropertyClass)
		{
			return;
		}

		auto IsPropertyAtStartOfBlacklistedClass = [&BlacklistedClasses](FNYProperty* CheckProperty) -> bool
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
		auto* Property = ParticipantClass->PropertyLink;
		while (Property != nullptr && !IsPropertyAtStartOfBlacklistedClass(Property))
		{
			if (Property->GetClass() == PropertyClass)
			{
				OutContainer.Add(Property->GetFName());
			}
			Property = Property->PropertyLinkNext;
		}
	}
};
#endif // NY_REFLECTION_HELPER
