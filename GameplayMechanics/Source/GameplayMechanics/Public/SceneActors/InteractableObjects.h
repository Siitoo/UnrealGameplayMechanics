// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractionInterface.h"
#include "InteractableObjects.generated.h"

UCLASS()
class GAMEPLAYMECHANICS_API AInteractableObjects : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableObjects();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual bool PrepareInteraction() override;
	virtual bool Interaction() override;
	virtual bool CancelInteraction() override;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* BoxTrigger;

	UPROPERTY(VisibleDefaultsOnly)
	class UWidgetComponent* InteractButtonWidget;
};
