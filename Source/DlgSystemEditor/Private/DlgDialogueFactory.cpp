// Fill out your copyright notice in the Description page of Project Settings.
#include "DlgDialogueFactory.h"

#include "DlgDialogue.h"

#define LOCTEXT_NAMESPACE "DlgSystem"

/////////////////////////////////////////////////////
// UDlgDialogueFactory
UDlgDialogueFactory::UDlgDialogueFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UDlgDialogue::StaticClass();
}

UObject* UDlgDialogueFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
											   UObject* Context, FFeedbackContext* Warn)
{
	UDlgDialogue* NewDialogue = NewObject<UDlgDialogue>(InParent, Class, Name, Flags | RF_Transactional);
	return NewDialogue;
}

#undef LOCTEXT_NAMESPACE
