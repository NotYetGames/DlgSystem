// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueK2Node_Select.h"

#include "EdGraphUtilities.h"
#include "KismetCompiler.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EditorStyleSet.h"

#include "DlgSystemEditorModule.h"
#include "DlgManager.h"
#include "DialogueBlueprintUtilities.h"

#define LOCTEXT_NAMESPACE "DlgK2Node_Select"

const FName UDialogueK2Node_Select::PIN_VariableName(TEXT("VariableName"));
const FName UDialogueK2Node_Select::PIN_DefaultValue(TEXT("DefaultValue"));

//////////////////////////////////////////////////////////////////////////
// FKCHandler_DialogueSelect
// TODO(vampy): Figure out why having the same name for a handler crashes things on linux and only some times in Windows
// For example if this is name FKCHandler_Select (like the normal K2Node_Select handler) the compiler confuses our node
// for that node. The name should be irrelevant right? the handler is used as value in a TMap, right????
class FKCHandler_DialogueSelect : public FNodeHandlingFunctor
{
protected:
	// Mutiple nodes possible? isn't this called only on this node
	TMap<UEdGraphNode*, FBPTerminal*> BoolTermMap;

public:
	FKCHandler_DialogueSelect(FKismetCompilerContext& InCompilerContext) : FNodeHandlingFunctor(InCompilerContext)
	{
	}

	void RegisterNets(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		FNodeHandlingFunctor::RegisterNets(Context, Node);
		const UDialogueK2Node_Select* SelectNode = CastChecked<UDialogueK2Node_Select>(Node);

		// Create the net for the return value manually as it's a special case Output Direction pin
		{
			UEdGraphPin* ReturnPin = SelectNode->GetReturnValuePin();
			FBPTerminal* Term = Context.CreateLocalTerminalFromPinAutoChooseScope(ReturnPin, Context.NetNameMap->MakeValidName(ReturnPin));
			Context.NetMap.Add(ReturnPin, Term);
		}

		// Create a terminal to determine if the compare was successful or not
		FBPTerminal* BoolTerm = Context.CreateLocalTerminal();
		BoolTerm->Type.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		BoolTerm->Source = Node;
		BoolTerm->Name = Context.NetNameMap->MakeValidName(Node) + TEXT("_CmpSuccess");
		BoolTermMap.Add(Node, BoolTerm);
	}

	void Compile(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		// Pseudocode of how this is compiled to:
		// We have N option pins - Options[N]
		//
		// IndexValue = ConditionTerm
		// ReturnValue = ReturnTerm
		// PrevIfNotStatement = null
		//
		// for Option in Options:
		//    OptionValue = Value of Option
		//    CallConditionFunctionStatement = AddStatement `BoolTerm = ConditionFunction(IndexValue, OptionValue)`
		//
		//    // where the previous statement jumps if it fails
		//    if PrevIfNotStatement is not null:
		//       PrevIfNotStatement.JumpTarget = CallConditionFunctionStatement
		//
		//    // the target is set above
		//    IfNotStatement = AddStatement `GoToTargetIfNot(BoolTerm, JumpTarget=null)`
		//
		//    // Add return option for this Option
		//    AddStatement `ReturnValue = OptionValue`
		//
		//   PrevIfNotStatement = IfNotStatement
		//   // add some goto statements that allows us to to safely exit the loop
		//
		// // point goto statements to a noop at the end

		// Cast the node and get all the input pins, the options we are selecting from
		UDialogueK2Node_Select* SelectNode = CastChecked<UDialogueK2Node_Select>(Node);
		const TArray<UEdGraphPin*> OptionPins = SelectNode->GetOptionPins();

		// Get the kismet term for the (Condition or Index) that will determine which option to use
		const UEdGraphPin* VariableNameConditionPin = FEdGraphUtilities::GetNetFromPin(SelectNode->GetVariableNamePin());
		FBPTerminal** ConditionTerm = Context.NetMap.Find(VariableNameConditionPin);

		// Get the kismet term for the return value
		const UEdGraphPin* ReturnPin = SelectNode->GetReturnValuePin();
		FBPTerminal** ReturnTerm = Context.NetMap.Find(ReturnPin);

		// Get the kismet term for the default value
		const UEdGraphPin* DefaultPin = FEdGraphUtilities::GetNetFromPin(SelectNode->GetDefaultValuePin());
		FBPTerminal** DefaultTerm = Context.NetMap.Find(DefaultPin);

		// Don't proceed if there is no return value or there is no selection
		if (ConditionTerm == nullptr || ReturnTerm == nullptr || DefaultTerm == nullptr)
		{
			return;
		}

		// Get the function that determines how the condition is computed
		UFunction* ConditionFunction = SelectNode->GetConditionalFunction();

		// Find the local boolean for use in the equality call function below (BoolTerm = result of EqualEqual_NameName)
		// Aka the result of the ConditionFunction
		FBPTerminal* BoolTerm = BoolTermMap.FindRef(SelectNode);

		// We need to keep a pointer to the previous IfNot statement so it can be linked to the next conditional statement
		FBlueprintCompiledStatement* PrevIfNotStatement = nullptr;

		// Keep an array of all the unconditional goto statements so we can clean up their jumps after the noop statement is created
		TArray<FBlueprintCompiledStatement*> GotoStatementList;

		// Loop through all the options
		const int32 OptionsNum = OptionPins.Num();
		for (int32 OptionIndex = 0; OptionIndex < OptionsNum; OptionIndex++)
		{
			UEdGraphPin* OptionPin = OptionPins[OptionIndex];

			// Create a CallFunction statement with the condition function from the Select Node class
			// The Previous option (PrevIfNotStatement) points to this CallConditionFunctionStatement
			{
				// This is our Condition function.
				FBlueprintCompiledStatement& CallConditionFunctionStatement = Context.AppendStatementForNode(Node);
				CallConditionFunctionStatement.Type = KCST_CallFunction;
				CallConditionFunctionStatement.FunctionToCall = ConditionFunction;
				CallConditionFunctionStatement.FunctionContext = nullptr;
				CallConditionFunctionStatement.bIsParentContext = false;

				// BoolTerm will be the return value of the condition statement
				CallConditionFunctionStatement.LHS = BoolTerm;

				// Compare index value == option value
				// The condition passed into the Select node
				CallConditionFunctionStatement.RHS.Add(*ConditionTerm);

				// Create a literal/constant for the current option pin
				FBPTerminal* LiteralTerm = Context.CreateLocalTerminal(ETerminalSpecification::TS_Literal);
				LiteralTerm->bIsLiteral = true;

				// Does the name of the input pin matches the VariableName Pin value?
				LiteralTerm->Type.PinCategory = UEdGraphSchema_K2::PC_Name;
				LiteralTerm->Name = OptionPin->PinName.ToString();

				// Compare against the current literal
				CallConditionFunctionStatement.RHS.Add(LiteralTerm);

				// If there is a previous IfNot statement, hook this one to that one for jumping
				if (PrevIfNotStatement)
				{
					CallConditionFunctionStatement.bIsJumpTarget = true;
					PrevIfNotStatement->TargetLabel = &CallConditionFunctionStatement;
				}
			}

			// Create a GotoIfNot statement using the BoolTerm from above as the condition
			FBlueprintCompiledStatement* IfNotStatement = &Context.AppendStatementForNode(Node);
			IfNotStatement->Type = KCST_GotoIfNot;
			IfNotStatement->LHS = BoolTerm;

			// Create an assignment statement
			// If the option matches, make the return (terminal) be the value of our option
			{
				FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
				AssignStatement.Type = KCST_Assignment;
				AssignStatement.LHS = *ReturnTerm;

				// Get the kismet terminal from the option pin
				UEdGraphPin* OptionPinToTry = FEdGraphUtilities::GetNetFromPin(OptionPin);
				FBPTerminal** OptionTerm = Context.NetMap.Find(OptionPinToTry);
				if (!OptionTerm)
				{
					Context.MessageLog.Error(*LOCTEXT("Error_UnregisterOptionPin", "Unregister option pin @@").ToString(), OptionPin);
					return;
				}
				AssignStatement.RHS.Add(*OptionTerm);
			}

			// Create an unconditional goto to exit the node
			FBlueprintCompiledStatement& GotoStatement = Context.AppendStatementForNode(Node);
			GotoStatement.Type = KCST_UnconditionalGoto;
			GotoStatementList.Add(&GotoStatement);

			// If this is the last IfNot statement, hook the next jump target to be the default value
			// as all the options are exhausted
			if (OptionIndex == OptionsNum - 1)
			{
				FBlueprintCompiledStatement& AssignStatement = Context.AppendStatementForNode(Node);
				AssignStatement.Type = KCST_Assignment;
				AssignStatement.bIsJumpTarget = true;
				AssignStatement.LHS = *ReturnTerm;
				AssignStatement.RHS.Add(*DefaultTerm);

				// Hook the IfNot statement's jump target to this assign statement
				IfNotStatement->TargetLabel = &AssignStatement;
			}

			PrevIfNotStatement = IfNotStatement;
		}

		// Create a noop to jump to so the unconditional goto statements can exit the node after successful assignment
		FBlueprintCompiledStatement& NopStatement = Context.AppendStatementForNode(Node);
		NopStatement.Type = KCST_Nop;
		NopStatement.bIsJumpTarget = true;

		// Loop through the unconditional goto statements and fix their jump targets
		for (FBlueprintCompiledStatement* GotoStatement : GotoStatementList)
		{
			GotoStatement->TargetLabel = &NopStatement;
		}
	}
};

UDialogueK2Node_Select::UDialogueK2Node_Select(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VariableType = EDlgVariableType::Int;
	AdvancedPinDisplay = ENodeAdvancedPins::NoPins;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UEdGraphNode interface
void UDialogueK2Node_Select::AllocateDefaultPins()
{
	bReconstructNode = false;

	RefreshVariablePinType();
	RefreshPinNames();
	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

	// Params for all the pins, only the index is changed.
	FCreatePinParams PinParams;

	// Create the return value
	{
		PinParams.Index = INDEX_PIN_Return;
		UEdGraphPin* ReturnPin = CreatePin(EGPD_Output, VariablePinType, UEdGraphSchema_K2::PN_ReturnValue, PinParams);
		ReturnPin->bDisplayAsMutableRef = false;
	}

	// Create the variable name pin, the one the selections are based on
	{
		PinParams.Index = INDEX_PIN_VariableName;
		UEdGraphPin* VariableNamePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, PIN_VariableName, PinParams);
		VariableNamePin->bDisplayAsMutableRef = false;
		VariableNamePin->PinToolTip = TEXT("The Index/Condition Name that tells what option value to use.");
		Schema->SetPinAutogeneratedDefaultValueBasedOnType(VariableNamePin);
	}

	// Create the default value pin
	{
		PinParams.Index = INDEX_PIN_Default;
		UEdGraphPin* DefaultPin = CreatePin(EGPD_Input, VariablePinType, PIN_DefaultValue, PinParams);
		DefaultPin->bDisplayAsMutableRef = false;
		DefaultPin->PinToolTip = TEXT("The default value used if the Variable Name does not match any of the options above");
		Schema->SetPinAutogeneratedDefaultValueBasedOnType(DefaultPin);
	}

	// Create the option pins at the end of the array
	for (const FName& PinName : PinNames)
	{
		// NOTE: NO PinParams
		UEdGraphPin* NewPin = CreatePin(EGPD_Input, VariablePinType, PinName);
		NewPin->bDisplayAsMutableRef = false;
		Schema->SetPinAutogeneratedDefaultValueBasedOnType(NewPin);
	}

	Super::AllocateDefaultPins();
}

void UDialogueK2Node_Select::PinTypeChanged(UEdGraphPin* Pin)
{
	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

	if (Pin != GetVariableNamePin())
	{
		// Set the return value
		UEdGraphPin* ReturnPin = GetReturnValuePin();

		// Recombine the sub pins back into the ReturnPin
		if (ReturnPin->SubPins.Num() > 0)
		{
			Schema->RecombinePin(ReturnPin->SubPins[0]);
		}
		ReturnPin->PinType = Pin->PinType;

		// Recombine all option pins back into their root
		TArray<UEdGraphPin*> OptionPins = GetOptionPins();
		for (UEdGraphPin* OptionPin : OptionPins)
		{
			// Recombine the sub pins back into the OptionPin
			if (OptionPin->ParentPin == nullptr && OptionPin->SubPins.Num() > 0)
			{
				Schema->RecombinePin(OptionPin->SubPins[0]);
			}
		}

		// Get the options again and set them
		OptionPins = GetOptionPins();
		for (UEdGraphPin* OptionPin : OptionPins)
		{
			if (OptionPin->PinType != Pin->PinType || OptionPin == Pin)
			{
				OptionPin->PinType = Pin->PinType;
			}

			if (!Schema->IsPinDefaultValid(OptionPin, OptionPin->DefaultValue, OptionPin->DefaultObject, OptionPin->DefaultTextValue).IsEmpty())
			{
				Schema->ResetPinToAutogeneratedDefaultValue(OptionPin);
			}
		}

		// Recombine default pin
		UEdGraphPin* DefaultPin = GetDefaultValuePin();
		if (DefaultPin->ParentPin == nullptr && DefaultPin->SubPins.Num() > 0)
		{
			Schema->RecombinePin(DefaultPin->SubPins[0]);
		}
		DefaultPin->PinType = Pin->PinType;

		bReconstructNode = true;
	}
}


void UDialogueK2Node_Select::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();

	if (bReconstructNode)
	{
		ReconstructNode();

		UBlueprint* Blueprint = GetBlueprint();
		if (!Blueprint->bBeingCompiled)
		{
			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
			Blueprint->BroadcastChanged();
		}
	}
}

FText UDialogueK2Node_Select::GetTooltipText() const
{
	return LOCTEXT("DlgSelectNodeTooltipInt", "Return the int variable based on the name");
}

FText UDialogueK2Node_Select::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("DlgSelectInt", "Select Dialogue Int");
}

FSlateIcon UDialogueK2Node_Select::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon(FEditorStyle::GetStyleSetName(), "GraphEditor.Select_16x");
	return Icon;
}
// End UEdGraphNode interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UK2Node Interface
FNodeHandlingFunctor* UDialogueK2Node_Select::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return static_cast<FNodeHandlingFunctor*>(new FKCHandler_DialogueSelect(CompilerContext));
}

bool UDialogueK2Node_Select::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const
{
	if (OtherPin && (OtherPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec))
	{
		OutReason = LOCTEXT("ExecConnectionDisallowed", "Cannot connect with Exec pin.").ToString();
		return true;
	}

	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

/** Determine if any pins are connected, if so make all the other pins the same type, if not, make sure pins are switched back to wildcards */
void UDialogueK2Node_Select::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);

	// Check for WildCard pins
	if (Pin != GetVariableNamePin())
	{
		// Grab references to all option pins and the return pin
		const TArray<UEdGraphPin*> OptionPins = GetOptionPins();
		const UEdGraphPin* ReturnPin = GetReturnValuePin();
		const UEdGraphPin* DefaultPin = GetDefaultValuePin();

		// See if this pin is one of the wildcard pins
		const bool bIsWildcardPin = (Pin == ReturnPin || Pin == DefaultPin || OptionPins.Find(Pin) != INDEX_NONE)
				&& Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard;

		// If the pin was one of the wildcards we have to handle it specially
		if (bIsWildcardPin)
		{
			// If the pin is linked, make sure the other wildcard pins match
			if (Pin->LinkedTo.Num() > 0)
			{
				const UEdGraphPin* LinkPin = Pin->LinkedTo[0];

				// Linked pin type differs, change this type, change type
				if (Pin->PinType != LinkPin->PinType)
				{
					Pin->PinType = LinkPin->PinType;
					PinTypeChanged(Pin);
				}
			}
		}
	}
}

void UDialogueK2Node_Select::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	// actions get registered under specific object-keys; the idea is that
	// actions might have to be updated (or deleted) if their object-key is
	// mutated (or removed)... here we use the node's class (so if the node
	// type disappears, then the action should go with it)
	UClass* ActionKey = GetClass();

	// to keep from needlessly instantiating a UBlueprintNodeSpawner, first
	// check to make sure that the registrar is looking for actions of this type
	// (could be regenerating actions for a specific asset, and therefore the
	// registrar would only accept actions corresponding to that asset)
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UDialogueK2Node_Select::GetMenuCategory() const
{
	return LOCTEXT("DlgGetMenuCategory", "Dialogue|Select");
}

void UDialogueK2Node_Select::PostReconstructNode()
{
	// After ReconstructNode we must be sure that no additional reconstruction is required
	bReconstructNode = false;

	UEdGraphPin* ReturnPin = GetReturnValuePin();

	// Wild card pin? set types depending on options
	const bool bFillTypeFromConnected = ReturnPin && (ReturnPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard);
	if (bFillTypeFromConnected)
	{
		check(VariablePinType == ReturnPin->PinType.PinCategory);
		FEdGraphPinType PinType = ReturnPin->PinType;

		// Determine from Return pin type connection
		if (ReturnPin->LinkedTo.Num() > 0)
		{
			PinType = ReturnPin->LinkedTo[0]->PinType;
		}
		else
		{
			// Determine from Default pin type connection
			const UEdGraphPin* DefaultPin = GetDefaultValuePin();
			if (DefaultPin && DefaultPin->LinkedTo.Num() > 0)
			{
				PinType = DefaultPin->LinkedTo[0]->PinType;
			}
			else
			{
				// Determine from one of the option pin types connections
				const TArray<UEdGraphPin*> OptionPins = GetOptionPins();
				for (const UEdGraphPin* Pin : OptionPins)
				{
					if (Pin && Pin->LinkedTo.Num() > 0)
					{
						PinType = Pin->LinkedTo[0]->PinType;
						break;
					}
				}
			}
		}

		ReturnPin->PinType = PinType;
		PinTypeChanged(ReturnPin);
	}

	Super::PostReconstructNode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own functions
UFunction* UDialogueK2Node_Select::GetConditionalFunction()
{
	// The IndexPin (select by type)  is always an String (FName), so only use that
	const FName FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, EqualEqual_NameName);

#if ENGINE_MINOR_VERSION >= 25
	return FindUField<UFunction>(UKismetMathLibrary::StaticClass(), FunctionName);
#else
	return FindField<UFunction>(UKismetMathLibrary::StaticClass(), FunctionName);
#endif
}

void UDialogueK2Node_Select::GetPrintStringFunction(FName& FunctionName, UClass** FunctionClass)
{
	FunctionName = GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, PrintWarning);
	*FunctionClass = UKismetSystemLibrary::StaticClass();
}

bool UDialogueK2Node_Select::RefreshPinNames()
{
	// Stop anything if the blueprint is loading, this can happen because we now have reference to blueprint UClasses (reflection system) from the UDlgDialogue
	if (!FDialogueBlueprintUtilities::IsBlueprintLoadedForGraphNode(this))
	{
		return false;
	}

	const FName ParticipantName = FDialogueBlueprintUtilities::GetParticipantNameFromNode(this);
	if (ParticipantName == NAME_None && VariableType != EDlgVariableType::SpeakerState)
	{
		return false;
	}

	TArray<FName> NewPinNames;
	switch (VariableType)
	{
		case EDlgVariableType::Float:
			UDlgManager::GetAllDialoguesFloatNames(ParticipantName, NewPinNames);
			break;

		case EDlgVariableType::Int:
			UDlgManager::GetAllDialoguesIntNames(ParticipantName, NewPinNames);
			break;

		case EDlgVariableType::Name:
			UDlgManager::GetAllDialoguesNameNames(ParticipantName, NewPinNames);
			break;

		case EDlgVariableType::SpeakerState:
			UDlgManager::GetAllDialoguesSpeakerStates(NewPinNames);
			break;

		default:
			unimplemented();
	}

	// Size changed, simply copy
	if (NewPinNames.Num() != PinNames.Num())
	{
		PinNames = NewPinNames;
		return true;
	}

	// Find any difference, if any
	for (int32 i = 0; i < NewPinNames.Num(); ++i)
	{
		if (NewPinNames[i] != PinNames[i])
		{
			PinNames = NewPinNames;
			return true;
		}
	}

	return false;
}
// End own functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDlgK2Node_SelectFloat
// float variant
UDialogueK2Node_SelectFloat::UDialogueK2Node_SelectFloat(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VariableType = EDlgVariableType::Float;
}

FText UDialogueK2Node_SelectFloat::GetTooltipText() const
{
	return LOCTEXT("DlgSelectNodeTooltipFloat", "Return the float variable based on the name");
}

FText UDialogueK2Node_SelectFloat::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("DlgSelectFloat", "Select Dialogue Float");
}


// name variant
UDialogueK2Node_SelectName::UDialogueK2Node_SelectName(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VariableType = EDlgVariableType::Name;
}

FText UDialogueK2Node_SelectName::GetTooltipText() const
{
	return LOCTEXT("DlgSelectNodeTooltipName", "Return the name variable based on the name");
}

FText UDialogueK2Node_SelectName::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("DlgSelectName", "Select Dialogue Name");
}


// speaker state variant
UDialogueK2Node_SelectOnSpeakerState::UDialogueK2Node_SelectOnSpeakerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VariableType = EDlgVariableType::SpeakerState;
}

FText UDialogueK2Node_SelectOnSpeakerState::GetTooltipText() const
{
	return LOCTEXT("DlgSelectNodeTooltipOnSpeakerState", "Return the selected input based on the speaker state");
}

FText UDialogueK2Node_SelectOnSpeakerState::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("DlgSelectOnSpeakerState", "Select Dialogue SpeakerState");
}

#undef LOCTEXT_NAMESPACE
