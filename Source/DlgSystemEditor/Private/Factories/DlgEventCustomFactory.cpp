// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEventCustomFactory.h"

#include "DialogueEditorUtilities.h"
#include "DlgEventCustom.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "DlgSystem"

/////////////////////////////////////////////////////
// UDlgEventCustomFactory
UDlgEventCustomFactory::UDlgEventCustomFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;

	// true if the associated editor should be opened after creating a new object.
	bEditAfterNew = false;
	SupportedClass = UDlgEventCustom::StaticClass();

	// Default class
	ParentClass = SupportedClass;
}

bool UDlgEventCustomFactory::ConfigureProperties()
{
	static const FText TitleText = FText::FromString(TEXT("Pick Parent Class for Custom Event"));
	ParentClass = nullptr;

	UClass* ChosenClass = nullptr;
	const bool bPressedOk = FDialogueEditorUtilities::PickChildrenOfClass(TitleText, ChosenClass, SupportedClass);
	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UDlgEventCustomFactory::FactoryCreateNew(
	UClass* Class,
	UObject* InParent,
	FName Name,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn
)
{
	// Something wrong happened
	if (ParentClass == nullptr || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), ParentClass ? FText::FromString(ParentClass->GetName()) : NSLOCTEXT("UnrealEd", "Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(NSLOCTEXT("UnrealEd", "CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{0}'."), Args));
		return nullptr;
	}

	// Create
	return FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		InParent,
		Name,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		NAME_None
	);
}

#undef LOCTEXT_NAMESPACE
