// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs/GeneratedEdge.h"
#include "Structs/GeneratedNode.h"
#include "Structs/GeneratedTriangles.h"
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

	UFUNCTION(BlueprintCallable)
	void MyPoisonDiskSamplingAlgorithm();
	bool IsMyCandidateValid(FVector2D Candidate, FVector2D SampleRegionSize, float CellSize, TArray<int> Grid);
	UFUNCTION(BlueprintCallable)
	void DelaunaryTriangulation();
	UFUNCTION(BlueprintCallable)
	void GeneratePaths();


	UFUNCTION(BlueprintCallable)
	void DrawDebugGrid();
	UFUNCTION(BlueprintCallable)
	void DrawDebugPoisonDisk();
	UFUNCTION(BlueprintCallable)
	void DrawDebugDelaunary();

	UFUNCTION(BlueprintCallable)
	void DrawDebugPathGenerated();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Disk Sampling Grid Generator")
	int Seed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Disk Sampling Grid Generator")
	float GridExtend;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Disk Sampling Grid Generator")
	float SphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Disk Sampling Grid Generator")
	int Iterations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Disk Sampling Grid Generator")
	int NumSampleBeforeRejection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poison Disk Sampling Grid Generator")
	bool bCheckWellGenerated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Map Generator")
	bool bDebugGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Map Generator")
	bool bDebugPoisonDisk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Map Generator")
	bool bDebugDelaunary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Map Generator")
	bool bDebugGeneratedPath;

	TArray<FVector2D> GeneratedPoints;

	TArray<int> Grid;

	TArray<FGeneratedTriangle> Triangles;

	TArray<FGeneratedEdge> Edges;

	TArray<FGeneratedNode> Paths;

	FRandomStream Random;

	UPROPERTY(EditAnywhere)
	int PathfindingIterations;

private:

	UPROPERTY(EditAnywhere)
	FVector2D StartPoint;

	UPROPERTY(EditAnywhere)
	FVector2D EndPoint;

};

