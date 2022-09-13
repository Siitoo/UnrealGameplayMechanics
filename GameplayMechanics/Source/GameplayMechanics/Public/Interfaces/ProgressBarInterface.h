// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProgressBarInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UProgressBarInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAMEPLAYMECHANICS_API IProgressBarInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual float GetCurrentHealth();
	virtual float GetHealthAsRatio();

	virtual float GetCurrentMana();
	virtual float GetManaAsRatio();
};
