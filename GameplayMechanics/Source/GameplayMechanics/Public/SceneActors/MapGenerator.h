// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapGenerator.generated.h"

UENUM()
enum NodeType
{
	NormalBattle = 0,
	BossBattle,
	FinalBattle,
	EventRoom,
	ShopRoom,
	RewardRoom,
	Default
};


USTRUCT()
struct FMapNode
{
	GENERATED_BODY()

	FMapNode();


public:
	TEnumAsByte<NodeType> Node = NodeType::Default;
	AActor* NodeActor;
};


UCLASS()
class GAMEPLAYMECHANICS_API AMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	void GenerateBaseMap();
	void GenerateRandomRooms();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generator Settings")
	int MapLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generator Settings")
	int MapDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generator Settings")
	int MaxMapConections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generator Settings")
	int DistanceBetweenNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generator Settings")
	int DistanceBetweenLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generator Settings")
	TSubclassOf<AActor> DefaultActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Probability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityNormalBattle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Probability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityBossBattle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Probability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityEventRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Probability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityShoopRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Probability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityRewardRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Probability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProbabilityNoRoom;

private:

	TArray<FMapNode> MapArray;
	FMapNode BaseNode;
};
