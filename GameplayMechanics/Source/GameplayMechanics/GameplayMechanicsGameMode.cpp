// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayMechanicsGameMode.h"
#include "GameplayMechanicsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGameplayMechanicsGameMode::AGameplayMechanicsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
