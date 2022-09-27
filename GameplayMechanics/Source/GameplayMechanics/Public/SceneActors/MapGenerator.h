// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapGenerator.generated.h"

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

	void PoisonDiskSamplingAlgorithm();
	bool IsCandidateValid(FVector2D Candidate, FVector2D SampleRegionSize, float CellSize, TArray<int> Grid);

	void MyPoisonDiskSamplingAlgorithm();
	bool IsMyCandidateValid(FVector2D Candidate, FVector2D SampleRegionSize, float CellSize, TArray<int> Grid);

	void DrawDebugStartEndPoints();
	void DrawDebugGrid();
	void DrawDebugPoisonDisk();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere)
	FVector StartPoint;

	UPROPERTY(EditAnywhere)
	FVector EndPoint;

	UPROPERTY(EditAnywhere)
	float HalfGridWeight;

	UPROPERTY(EditAnywhere)
	int NumSampleBeforeRejection;

	UPROPERTY(EditAnywhere)
	float SphereRadius;

	UPROPERTY(EditAnywhere)
	int Seed;

	UPROPERTY(EditAnywhere)
	int Iterations;

	TArray<FVector2D> GeneratedPoints;

	TArray<int> Grid;
};

