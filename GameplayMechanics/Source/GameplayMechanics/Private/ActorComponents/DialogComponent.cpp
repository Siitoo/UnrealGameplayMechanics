// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/DialogComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../GameplayMechanicsCharacter.h"

// Sets default values for this component's properties
UDialogComponent::UDialogComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	BoxTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBoxComponent"));
	BoxTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxTrigger->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	BoxTrigger->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	BoxTrigger->SetBoxExtent(FVector(50.f, 50.f, 64.f));
	BoxTrigger->SetRelativeLocation(FVector(70.f, 0.f, 0.f));

	//AIDialogController = CreateDefaultSubobject<AAIController>(TEXT("Dialog Controller"));

	BehaviorTree = CreateDefaultSubobject<UBehaviorTree>(TEXT("BehaviorTree"));

	Blackboard = CreateDefaultSubobject<UBlackboardData>(TEXT("Blackboard"));

	DialogWidget = CreateDefaultSubobject<UUserWidget>(TEXT("Dialog Widget"));

	bDialogTriggered = false;
	
}

// Called when the game starts
void UDialogComponent::BeginPlay()
{
	Super::BeginPlay();
	
	FAttachmentTransformRules TransformRules(EAttachmentRule::KeepRelative, false);
	BoxTrigger->AttachToComponent(GetOwner()->GetRootComponent(), TransformRules);

	if (DialogWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NO DIALOG WIDGET CLASS SELECTED!"));
	}
	else
	{
		DialogWidget->SetVisibility(ESlateVisibility::Hidden);
	
	}
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	NPCController = Cast<AAIController>(World->SpawnActor(AIDialogController));

	//NPCController = Cast<AAIController>(AIDialogController);

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
	UE_LOG(LogTemp, Warning, TEXT("Preparing Dialog"));
	return false;
}

bool UDialogComponent::Interaction()
{
	UWorld* World = GetWorld();
	DialogWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	AGameplayMechanicsCharacter* MainPlayer = Cast<AGameplayMechanicsCharacter>(PlayerCharacter);

	if (bDialogTriggered)
	{
		UFunction* Func = DialogWidget->GetClass()->FindFunctionByName(FName("NextDialog"));

		if (Func != nullptr)
		{
			DialogWidget->ProcessEvent(Func, nullptr);
		}
	}
	else
	{
		MainPlayer->StartDialog(DialogWidget);
		NPCController->RunBehaviorTree(BehaviorTree);

		UBlackboardComponent* BlackBoardComponent;
		NPCController->UseBlackboard(Blackboard, BlackBoardComponent);
		
		BlackBoardComponent->SetValueAsObject(FName("DialogWidget"), DialogWidget);
		
		bDialogTriggered = true;
	}
	return false;
}

bool UDialogComponent::CancelInteraction()
{
	if (bDialogTriggered)
	{
		UWorld* World = GetWorld();

		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
		AGameplayMechanicsCharacter* MainPlayer = Cast<AGameplayMechanicsCharacter>(PlayerCharacter);
		MainPlayer->StopDialog(DialogWidget);

		bDialogTriggered = false;
		DialogWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	return false;
}

