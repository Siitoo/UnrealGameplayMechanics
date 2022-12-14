// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InteractionInterface.h"
#include "DialogComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEPLAYMECHANICS_API UDialogComponent : public UActorComponent, public IInteractionInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDialogComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool PrepareInteraction() override;
	bool Interaction() override;

	UFUNCTION(BlueprintCallable)
	bool CancelInteraction() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AIController")
	TSubclassOf<class AAIController> AIDialogController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BehaviourTree")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	class UBlackboardData* Blackboard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DialogWidget")
	class UUserWidget* DialogWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InteractableBox")
	class UBoxComponent* BoxTrigger;

private:

	bool bDialogTriggered;

	AAIController* NPCController;

};
