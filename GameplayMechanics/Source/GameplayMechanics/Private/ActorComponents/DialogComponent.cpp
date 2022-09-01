// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/DialogComponent.h"
#include "AIController.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"

// Sets default values for this component's properties
UDialogComponent::UDialogComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBoxComponent"));
	BoxTrigger->SetBoxExtent(FVector(50.f, 50.f, 50.f));
	BoxTrigger->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	AIDialogController = CreateDefaultSubobject<AAIController>(TEXT("Dialog Controller"));

	DialogWidget = CreateDefaultSubobject<UWidgetComponent>("Dialog Widget");
	
}

// Called when the game starts
void UDialogComponent::BeginPlay()
{
	Super::BeginPlay();

	DialogWidget->SetVisibility(false);

	// ...
}

// Called every frame
void UDialogComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UDialogComponent::PrepareInteraction()
{
	return false;
}

bool UDialogComponent::Interaction()
{
	DialogWidget->SetVisibility(true);
	return false;
}

bool UDialogComponent::CancelInteraction()
{
	return false;
}

