// Fill out your copyright notice in the Description page of Project Settings.


#include "SceneActors/MapGenerator.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
// Sets default values
AMapGenerator::AMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//DefaultActor = CreateDefaultSubobject<AActor>(TEXT("Default Node Actor"));

	MapLayers = 15;
	MapDensity = 7;
	MaxMapConections = 4;
	DistanceBetweenNodes = 50;
	DistanceBetweenLayers = 100;

	ProbabilityNormalBattle = 0.2f;
	ProbabilityBossBattle = 0.05f;
	ProbabilityEventRoom = 0.1f;
	ProbabilityRewardRoom = 0.1;
	ProbabilityShoopRoom = 0.1;
	ProbabilityNoRoom = 0.3;
}

// Called when the game starts or when spawned
void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	GenerateBaseMap();
	GenerateRandomRooms();
}


// Called every frame
void AMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMapGenerator::GenerateBaseMap()
{
	const int MaxNodes = MapLayers * MapDensity;
	MapArray.Init(BaseNode, MaxNodes);

	UWorld* World = GetWorld();

	for (int Index = 0; Index < MaxNodes; ++Index)
	{
		const int Column = Index / MapLayers;
		FVector Location(0.f,0.f,0.f);

		Location.X = ((Index + MapLayers * Column) % MapLayers) * DistanceBetweenNodes;
		Location.Y = ((Index + MapLayers * Column) / MapLayers) * DistanceBetweenLayers;

		FRotator Rotator = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParameters;

		MapArray[Index].NodeActor = World->SpawnActor<AActor>(DefaultActor, Location, Rotator);
	}
}

void AMapGenerator::GenerateRandomRooms()
{
	const float MaxProbabilities = ProbabilityNormalBattle + ProbabilityBossBattle + ProbabilityEventRoom
		+ ProbabilityRewardRoom + ProbabilityShoopRoom + ProbabilityNoRoom;

	const int MaxNodes = MapLayers * MapDensity;

	float MaxProbabilityBossBattle = ProbabilityBossBattle + ProbabilityNormalBattle;
	float MaxProbabilityEventRoom = MaxProbabilityBossBattle + ProbabilityEventRoom;
	float MaxProbabilityRewardRoom = ProbabilityEventRoom + ProbabilityRewardRoom;
	float MaxProbabilityShoopRoom = MaxProbabilityRewardRoom + ProbabilityShoopRoom;;

	float RandomValue = 0.0f;

	for (int Index = 0; Index < MaxNodes; ++Index)
	{
		UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(MapArray[Index].NodeActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

		if (StaticMesh != nullptr)
		{
			RandomValue = FMath::RandRange(0.f, MaxProbabilities);
			
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(StaticMesh->GetMaterial(0), nullptr);
			
			if (Mat != nullptr)
			{
				if (RandomValue < ProbabilityNormalBattle) //Normal Battle
				{
					Mat->SetVectorParameterValue("Base Color", FVector4(0.0f, 1.0f, 0.0f, 1.0f));
					MapArray[Index].Node = NodeType::NormalBattle;
				}
				else if (RandomValue >= ProbabilityNormalBattle && RandomValue < MaxProbabilityBossBattle) //Boss Battle
				{
					Mat->SetVectorParameterValue("Base Color", FVector4(1.0f, 0.0f, 0.0f, 1.0f));
					MapArray[Index].Node = NodeType::BossBattle;
				}
				else if (RandomValue >= MaxProbabilityBossBattle && RandomValue < MaxProbabilityEventRoom) //Event Room
				{
					Mat->SetVectorParameterValue("Base Color", FVector4(0.0f, 0.0f, 1.0f, 1.0f));
					MapArray[Index].Node = NodeType::EventRoom;
				}
				else if (RandomValue >= MaxProbabilityEventRoom && RandomValue < MaxProbabilityRewardRoom) //Reward Room
				{
					Mat->SetVectorParameterValue("Base Color", FVector4(0.5f, 0.5f, 0.1f, 1.0f));
					MapArray[Index].Node = NodeType::RewardRoom;
				}
				else if (RandomValue >= MaxProbabilityRewardRoom && RandomValue < MaxProbabilityShoopRoom) //Shoop Room
				{
					Mat->SetVectorParameterValue("Base Color", FVector4(1.0f, 1.0f, 1.0f, 1.0f));
					MapArray[Index].Node = NodeType::ShopRoom;
				}
				else //No Room
				{
					Mat->SetVectorParameterValue("Base Color", FVector4(0.0f, 0.0f, 0.0f, 1.0f));
				}

				StaticMesh->SetMaterial(0, Mat);

			}
		}
	}
}

FMapNode::FMapNode()
{
	NodeActor = nullptr;
}
