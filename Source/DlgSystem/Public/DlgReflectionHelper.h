// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "PropertyPortFlags.h"

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

	static void ExecuteFunction(UObject* ParticipantObject, const FName MethodName, FName variableInfo)
	{
		TArray<uint8> methodBuffer = TArray<uint8>();
		TArray<UProperty*> methodResults;
		ExecuteFunction(ParticipantObject, MethodName, variableInfo, methodResults, methodBuffer);

		//!!destructframe see also UObject::ProcessEvent
		for (UProperty* property : methodResults)
		{
			property->DestroyValue_InContainer(methodBuffer.GetData());
		}
	}

	/** This function allows you to execute another function. NOTE: Be sure to destroy the outparams!*/
	static void ExecuteFunction(UObject* ParticipantObject, const FName MethodName, FName variableInfo, TArray<UProperty*>& methodParams, TArray<uint8>& outBuffer)
	{
		if (!IsValid(ParticipantObject))
		{
			return;
		}

		UFunction* function = ParticipantObject->FindFunction(MethodName);
		if (function == nullptr)
		{
			return;
		}

		const FString command = FString::Printf(TEXT("%s %s"), *MethodName.ToString(), *variableInfo.ToString());
		const TCHAR* Str = *command;

		// Find an exec function.
		FString MsgStr;
		if (!FParse::Token(Str, MsgStr, true))
		{
			return;
		}
		if (nullptr == function)
		{
			return;
		}

		UProperty* LastParameter = nullptr;

		// find the last parameter
		for (TFieldIterator<UProperty> It(function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
		{
			LastParameter = *It;
		}

		// Parse all function parameters.
		uint8* Parms = static_cast<uint8*>(FMemory_Alloca(function->ParmsSize));
		FMemory::Memzero(Parms, function->ParmsSize);

		for (TFieldIterator<UProperty> It(function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			UProperty* LocalProp = *It;
			checkSlow(LocalProp);
			if (!LocalProp->HasAnyPropertyFlags(CPF_ZeroConstructor))
			{
				LocalProp->InitializeValue_InContainer(Parms);
			}
		}

		const uint32 ExportFlags = PPF_None;
		bool bFailed = false;
		int32 NumParamsEvaluated = 0;
		int commandVariableCount = 0;
		for (TFieldIterator<UProperty> It(function); It && (It->PropertyFlags & (CPF_Parm)) == CPF_Parm; ++It, NumParamsEvaluated++)
		{
			UProperty* PropertyParam = *It;
			checkSlow(PropertyParam); // Fix static analysis warning
			if (NumParamsEvaluated == 0 && ParticipantObject)
			{
				UObjectPropertyBase* Op = dynamic_cast<UObjectPropertyBase*>(*It);
				if (Op && ParticipantObject->IsA(Op->PropertyClass))
				{
					// First parameter is implicit reference to object executing the command.
					Op->SetObjectPropertyValue(Op->ContainerPtrToValuePtr<uint8>(Parms), ParticipantObject);
					continue;
				}
			}

			if (PropertyParam->HasAnyPropertyFlags(CPF_OutParm))
			{
				continue;
			}

			// Keep old string around in case we need to pass the whole remaining string
			const TCHAR* RemainingStr = Str;

			// Parse a new argument out of Str
			FString ArgStr;
			FParse::Token(Str, ArgStr, true);

			if (ArgStr.Len() > 0)
			{
				commandVariableCount++;
			}

			// if ArgStr is empty but we have more params to read parse the function to see if these have defaults, if so set them
			bool bFoundDefault = false;
			bool bFailedImport = false;

			if (variableInfo == "")
			{
				continue;
			}


#if WITH_EDITOR
			if (!FCString::Strcmp(*ArgStr, TEXT("")))
			{
				const FName DefaultPropertyKey(*(FString(TEXT("CPP_Default_")) + PropertyParam->GetName()));
				const FString& PropertyDefaultValue = function->GetMetaData(DefaultPropertyKey);
				if (!PropertyDefaultValue.IsEmpty())
				{
					bFoundDefault = true;

					const TCHAR* Result = It->ImportText(*PropertyDefaultValue, It->ContainerPtrToValuePtr<uint8>(Parms), ExportFlags, nullptr);
					bFailedImport = (Result == nullptr);
				}
			}
#endif

			if (!bFoundDefault)
			{
				// if this is the last string property and we have remaining arguments to process, we have to assume that this
				// is a sub-command that will be passed to another exec (like "cheat giveall weapons", for example). Therefore
				// we need to use the whole remaining string as an argument, regardless of quotes, spaces etc.
				if (PropertyParam == LastParameter && PropertyParam->IsA<UStrProperty>() && FCString::Strcmp(Str, TEXT("")) != 0)
				{
					ArgStr = FString(RemainingStr).TrimStart();
				}

				const TCHAR* Result = It->ImportText(*ArgStr, It->ContainerPtrToValuePtr<uint8>(Parms), ExportFlags, nullptr);
				bFailedImport = (Result == nullptr);
			}

			if (bFailedImport)
			{
				break;
			}
		}

		int inputParamCount = 0;
		// We need to make sure that all input params have been filled with information so we count the input params
		// Then compare that with the command params (the amount of variables in that string)
		for (TFieldIterator<UProperty> It(function); It && It->HasAnyPropertyFlags(CPF_OutParm) == false && It->HasAllPropertyFlags(CPF_Parm); ++It)
		{
			inputParamCount++;
		}

		// The amount of variables we still need to parse, do not add up with the input params of this functions
		if (inputParamCount != commandVariableCount)
		{
			UE_LOG(LogDlgSystemReflectionHelper, Warning,
				TEXT("The amount of required input parameters (%s) does not add up with the amount of variables given (%s). If this is intended, you can ignore this warning."),
				*FString::FromInt(inputParamCount), *FString::FromInt(commandVariableCount));
		}

		if (!bFailed)
		{
			ParticipantObject->ProcessEvent(function, Parms);
		}

		outBuffer.AddUninitialized(function->ParmsSize);
		FMemory::Memcpy(outBuffer.GetData(), Parms, function->ParmsSize);

		for (UProperty* property = function->PropertyLink; property != nullptr; property = property->PropertyLinkNext)
		{
			if (property == nullptr)
			{
				continue;
			}

			if (property->HasAnyPropertyFlags(CPF_Parm) == false)
			{
				continue;
			}

			methodParams.Add(property);
		}
	}

	/** Attempts to get the property MethodName from ParticipantObject then executes that method */
	template<typename ReturnType, typename PropertyType>
	static ReturnType GetMethodResult(UObject* ParticipantObject, const FName MethodName, FName variableInfo);


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

	/** Gathers the name of all methods in ParticipantClass */
	template <typename ContainerType>
	static void GetMethodNames(const UClass* ParticipantClass, ContainerType& OutContainer, UClass* returnType, const bool requiresInputParams);
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

template <typename ReturnType, typename PropertyType>
ReturnType UDlgReflectionHelper::GetMethodResult(UObject* ParticipantObject, const FName MethodName, FName variableInfo)
{
	TArray<uint8> methodBuffer = TArray<uint8>();
	TArray<UProperty*> methodResults;
	ExecuteFunction(ParticipantObject, MethodName, variableInfo, methodResults, methodBuffer);

	UFunction* relatedFunction = ParticipantObject->FindFunction(MethodName);

	if (relatedFunction == nullptr)
	{
		UE_LOG(LogDlgSystemReflectionHelper, Error, TEXT("Couldn't find the function %s on %s"), *MethodName.ToString(), *ParticipantObject->GetName());
		return ReturnType{};
	}

	if (methodBuffer.GetData() == nullptr ||
		methodResults.Num() == 0)
	{
		UE_LOG(LogDlgSystemReflectionHelper, Error, TEXT("Couldn't retrieve information from the function %s"), *MethodName.ToString())
			return ReturnType{};
	}

	ReturnType returnValue{};

	for (UProperty* property = relatedFunction->PropertyLink; property != nullptr; property = property->PropertyLinkNext)
	{
		// We need to make sure this is an out param
		if (property->HasAnyPropertyFlags(CPF_OutParm) == false)
		{
			continue;
		}

		// Gotta make sure it's the same return class type.
		const PropertyType* castedProperty = Cast<PropertyType>(property);
		if (castedProperty == nullptr)
		{
			continue;
		}

		returnValue = *property->ContainerPtrToValuePtr<ReturnType>(methodBuffer.GetData());
		break;
	}

	//!!destructframe see also UObject::ProcessEvent
	for (UProperty* property : methodResults)
	{
		property->DestroyValue_InContainer(methodBuffer.GetData());
	}

	return returnValue;
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


template <typename ContainerType>
void UDlgReflectionHelper::GetMethodNames(const UClass* ParticipantClass, ContainerType& OutContainer, UClass* returnType, const bool requiresInputParams)
{
	if (!IsValid(ParticipantClass))
	{
		return;
	}

	for (TFieldIterator<UFunction> function(ParticipantClass); function; ++function)
	{
		// If we have no paramaters, but we do require them: continue
		if (function->NumParms == 0 &&
			(requiresInputParams ||
				returnType != nullptr))
		{
			continue;
		}

		// If we have paramaters and dont require them at all: continue
		if (function->NumParms > 0 &&
			requiresInputParams == false &&
			returnType == nullptr)
		{
			continue;
		}

		// This function has out params, but we do not care for those at all: continue
		if (function->HasAnyFunctionFlags(FUNC_HasOutParms) &&
			returnType == nullptr)
		{
			continue;
		}

		// This function has no in/out params and we do not care about them. Add it to the list and continue
		if (function->NumParms == 0 &&
			function->HasAnyFunctionFlags(FUNC_HasOutParms) == false &&
			requiresInputParams == false &&
			returnType == nullptr)
		{
			OutContainer.Add(function->GetFName());
			continue;
		}

		// This function has input params, which we require and no output params which we dont care about
		// Add to the list and continue
		if (function->NumParms > 0 &&
			function->HasAnyFunctionFlags(FUNC_HasOutParms) == false &&
			requiresInputParams &&
			returnType == nullptr)
		{
			OutContainer.Add(function->GetFName());
			continue;
		}

		// We need to make sure this function only has output params before we can add it to the list
		if (function->NumParms > 0 &&
			function->HasAnyFunctionFlags(FUNC_HasOutParms) &&
			requiresInputParams == false &&
			returnType != nullptr)
		{
			bool hasOutParams = false;
			for (TFieldIterator<UProperty> PropIt(*function); PropIt && PropIt->HasAnyPropertyFlags(CPF_OutParm); ++PropIt)
			{
				if (PropIt->GetClass() != returnType)
				{
					continue;
				}

				hasOutParams = true;
				break;
			}

			if (hasOutParams)
			{
				OutContainer.Add(function->GetFName());
				continue;
			}
		}

		// We need to make sure this function has input and output params before we can add it to the list
		if (function->NumParms > 0 &&
			function->HasAnyFunctionFlags(FUNC_HasOutParms) &&
			requiresInputParams &&
			returnType != nullptr)
		{
			// We already know this function has output params, so we don't need to check for that
			bool hasInputParams = false;
			bool hasOutputParams = false;
			for (TFieldIterator<UProperty> PropIt(*function); PropIt; ++PropIt)
			{
				// No need to keep checking, we fine
				if (hasInputParams &&
					hasOutputParams)
				{
					break;
				}

				// We need to make sure our output param is the correct class
				if (PropIt->HasAnyPropertyFlags(CPF_OutParm) &&
					PropIt->GetClass() == returnType &&
					hasOutputParams == false)
				{
					hasOutputParams = true;
					continue;
				}

				// In case we haven't looped over all params yet, we need to keep going
				if (hasInputParams &&
					hasOutputParams == false)
				{
					continue;
				}

				hasInputParams = true;
			}

			if (hasInputParams &&
				hasOutputParams)
			{
				OutContainer.Add(function->GetFName());
				continue;
			}
		}

		// Support non-blueprint implemented functions
		UProperty* returnProperty = function->GetReturnProperty();

		if (returnProperty == nullptr)
		{
			continue;
		}

		// The function needs to have a boolean, otherwise we cannot use this for an enter condition
		if (returnProperty->GetClass() == returnType)
		{
			OutContainer.Add(function->GetFName());
		}

	}
}
