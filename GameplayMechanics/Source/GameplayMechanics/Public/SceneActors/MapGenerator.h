// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapGenerator.generated.h"

USTRUCT()
struct FGeneratedTriangle
{
	GENERATED_BODY()

	FGeneratedTriangle(){ };
	FGeneratedTriangle(FVector2D v1, FVector2D v2, FVector2D v3)
	{
		Vertex1 = v1;
		Vertex2 = v2;
		Vertex3 = v3;
	};

	bool CircumCircleContains(const FVector2D v) const;

public:
	FVector2D Vertex1, Vertex2, Vertex3;
	bool bIsBad = false;
};

USTRUCT()
struct FGeneratedEdge
{
	GENERATED_BODY()

	FGeneratedEdge() { };
	FGeneratedEdge(FVector2D v1, FVector2D v2)
	{
		StartPoint = v1;
		EndPoint = v2;
	};

public:
	FVector2D StartPoint, EndPoint;
	bool bIsBad = false;
	
};

USTRUCT()
struct FPath
{
	GENERATED_BODY()

	FPath() { };


public:
	FVector2D Position;
	TArray<FVector2D> LinkedPositions;

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

	void MyPoisonDiskSamplingAlgorithm();
	bool IsMyCandidateValid(FVector2D Candidate, FVector2D SampleRegionSize, float CellSize, TArray<int> Grid);

	void DelaunaryTriangulation();

	void GeneratePaths();

	void DrawDebugStartEndPoints();
	void DrawDebugGrid();
	void DrawDebugPoisonDisk();
	void DrawDebugDelaunary();
	void DrawDebugPathGenerated();

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

	FGeneratedTriangle SuperTriangle;
	TArray<FGeneratedTriangle> Triangles;

	TArray<FGeneratedEdge> Edges;

	TArray<FPath> Paths;
	FRandomStream Random;
};

