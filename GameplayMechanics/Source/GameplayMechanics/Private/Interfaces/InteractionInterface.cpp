// Fill out your copyright notice in the Description page of Project Settings.


#include "Interfaces/InteractionInterface.h"

// Add default functionality here for any IInteractionInterface functions that are not pure virtual.
bool IInteractionInterface::PrepareInteraction()
{
	UE_LOG(LogTemp, Warning, TEXT("I'm the interface, preparing the interaction!"));
	return false;
}

bool IInteractionInterface::Interaction()
{
	UE_LOG(LogTemp, Warning, TEXT("I'm the interface, interaction!"));
	return false;
}

bool IInteractionInterface::CancelInteraction()
{
	UE_LOG(LogTemp, Warning, TEXT("I'm the interface, cancel interaction!"));
	return false;
}