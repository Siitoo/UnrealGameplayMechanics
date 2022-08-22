// Fill out your copyright notice in the Description page of Project Settings.

#include "SceneActors/InteractableObjects.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AInteractableObjects::AInteractableObjects()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBoxComponent"));
	BoxTrigger->SetupAttachment(GetRootComponent());
	BoxTrigger->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	BoxTrigger->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	InteractButtonWidget = CreateDefaultSubobject<UWidgetComponent>("ButtonWidget");
	InteractButtonWidget->SetupAttachment(BoxTrigger);
}

// Called when the game starts or when spawned
void AInteractableObjects::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractableObjects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AInteractableObjects::PrepareInteraction()
{
	return false;
}

bool AInteractableObjects::Interaction()
{
	return false;
}

bool AInteractableObjects::CancelInteraction()
{
	return false;
}