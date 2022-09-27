// Fill out your copyright notice in the Description page of Project Settings.

#include "SceneActors/MapGenerator.h"
#include "DrawDebugHelpers.h"
#include "GenericPlatform/GenericPlatformMath.h"

// Sets default values
AMapGenerator::AMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StartPoint = FVector(0.0f);
	EndPoint = FVector(50.0f, 0.0f, 0.0f);
	HalfGridWeight = 25.f;
	SphereRadius = 5.0f;
	NumSampleBeforeRejection = 1;
	Seed = 123456789;
	Iterations = 50000;
}

// Called when the game starts or when spawned
void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	MyPoisonDiskSamplingAlgorithm();
}

bool AMapGenerator::IsMyCandidateValid(FVector2D Candidate, FVector2D RegionSize, float CellSize, TArray<int> GridCells)
{
	if (Candidate.X >= 0 && Candidate.X < RegionSize.X && Candidate.Y >= 0 && Candidate.Y < RegionSize.Y)
	{
		int LocationX = (int)(Candidate.X / CellSize);
		int LocationY = (int)(Candidate.Y / CellSize);

		//To search 5 by 5 around the cell
		int SearchStartX = FMath::Max(0, LocationX - 2);
		int SearchEndX = FMath::Min(LocationX + 2, RegionSize.X/ CellSize);

		int SearchStartY = FMath::Max(0, LocationY - 2);
		int SearchEndY = FMath::Min(LocationY + 2, RegionSize.Y/ CellSize);

		for (int X = SearchStartX; X <= SearchEndX; ++X)
		{
			for (int Y = SearchStartY; Y <= SearchEndY; ++Y)
			{
				int Index = ceil(RegionSize.X/CellSize) * X + Y;
				int PointIndex = GridCells[Index] - 1;

				if (PointIndex != -1)
				{
					FVector2D Point = GeneratedPoints[PointIndex];
					FVector2D Dist = Candidate - Point;
					float SqrtDistance = Dist.SizeSquared();

					if (SqrtDistance < SphereRadius * SphereRadius)
					{
						return false;
					}

				}

			}
		}

		return true;
	}

	return false;
}


void AMapGenerator::MyPoisonDiskSamplingAlgorithm()
{
	GeneratedPoints.Reset(0);
	Grid.Reset(0);

	FRandomStream Random = FRandomStream(Seed);
	float PISimplified = 3.141592654f;

	FVector2D RegionSize = FVector2D(EndPoint.X - StartPoint.X, HalfGridWeight * 2.f);
	float CellSize = SphereRadius / FMath::Sqrt(2.0f);
	int MaxGridCellsX = ceil(RegionSize.X/CellSize);
	int MaxGridCellsY = ceil(RegionSize.Y/CellSize);
	int GridSize = MaxGridCellsX * MaxGridCellsY;

	
	Grid.SetNumZeroed(GridSize);

	TArray<FVector2D> SpawnPoints;

	//StartedPoint, we try with the middle point
	FVector2D StartedPoint = RegionSize / 2.0f;
	SpawnPoints.Add(StartedPoint);

	while (SpawnPoints.Num() > 0 && Iterations > 0)
	{
		int RandomSpawnIndex = Random.RandRange(0, SpawnPoints.Num() - 1);
		FVector2D SpawnRandomPoint = SpawnPoints[RandomSpawnIndex];

		bool bCandidateAccepted = false;

		for (int Index = 0; Index < NumSampleBeforeRejection; ++Index)
		{
			float Angle = Random.FRand() * PISimplified * 2;
			FVector2D Direction = FVector2D(FMath::Sin(Angle), FMath::Cos(Angle));
			FVector2D CandidatePoint = SpawnRandomPoint + Direction * Random.RandRange(SphereRadius, SphereRadius * 2.0f);

			if (IsMyCandidateValid(CandidatePoint, RegionSize, CellSize, Grid))
			{
				GeneratedPoints.Add(CandidatePoint);
				SpawnPoints.Add(CandidatePoint);

				int LocationX = (int)(CandidatePoint.X / CellSize);
				int LocationY = (int)(CandidatePoint.Y / CellSize);

				int GridIndex = MaxGridCellsX * LocationX + LocationY;
				Grid[GridIndex] = GeneratedPoints.Num();
				bCandidateAccepted = true;
				break;
			}

		}

		if (!bCandidateAccepted)
		{
			SpawnPoints.RemoveAt(RandomSpawnIndex);
		}
		Iterations -= 1;
	}

	if (!SpawnPoints.IsEmpty() && SpawnPoints[0] == StartedPoint)
	{
		SpawnPoints.RemoveAt(0);
	}

	for (int Index = 0; Index < SpawnPoints.Num() - 2; ++Index)
	{
		FVector2D Pos = SpawnPoints[Index];

		for (int NextIndex = Index + 1; NextIndex < SpawnPoints.Num(); ++NextIndex)
		{
			FVector2D NextPos = SpawnPoints[NextIndex];

			FVector2D Dist = NextPos - Pos;
			float SqrtDistance = Dist.SizeSquared();

			if (SqrtDistance < SphereRadius * SphereRadius)
			{
				UE_LOG(LogTemp, Warning, TEXT("Bad Disck Noise Sample"));
			}
		}
	}
}

void AMapGenerator::DrawDebugStartEndPoints()
{
	UWorld* World = GetWorld();
	DrawDebugSphere(World, StartPoint, 1.0f, 10, FColor::Green);
	DrawDebugSphere(World, EndPoint, 1.0f, 10, FColor::Blue);
}

void AMapGenerator::DrawDebugGrid()
{
	UWorld* World = GetWorld();
	FVector BoxCenter = EndPoint - StartPoint;

	if (StartPoint.IsZero())
	{
		BoxCenter /= 2.0f;
	}

	DrawDebugBox(World, BoxCenter, FVector(BoxCenter.X, HalfGridWeight * 2.f, 0.1f), FColor::Green);

	FVector2D RegionSize = FVector2D(EndPoint.X - StartPoint.X, HalfGridWeight * 2.f);
	float CellSize = SphereRadius / FMath::Sqrt(2.0f);
	int MaxGridCellsX = ceil(RegionSize.X / CellSize);
	int MaxGridCellsY = ceil(RegionSize.Y / CellSize);

	for (int Index = 0; Index < MaxGridCellsX; ++Index)
	{
		FVector Start = StartPoint;
		Start.Y = Index*CellSize;
		FVector End = EndPoint;
		End.Y = Index * CellSize;
		DrawDebugLine(World, Start, End, FColor::Black);
	}

	for (int Index = 0; Index < MaxGridCellsY; ++Index)
	{
		FVector Start = FVector(0.0f);
		Start.X = Index * CellSize;
		FVector End = FVector(0.0f,RegionSize.Y,0.0f);
		End.X = Index * CellSize;
		DrawDebugLine(World, Start, End, FColor::Black);
	}

	for (int X = 0; X < MaxGridCellsX; ++X)
	{
		for (int Y = 0; Y < MaxGridCellsY; ++Y)
		{
			int GridIndex = MaxGridCellsX * X + Y;

			FColor Color = FColor::Black;

			if (Grid[GridIndex] - 1 != -1)
			{
				Color = FColor::Magenta;
			}

			FVector Position = FVector(X * CellSize + CellSize/2.0f, Y * CellSize + CellSize / 2.0f, 0.0f);

			DrawDebugSphere(World, Position, 0.2, 1.0f, Color);
		}
	}

}

void AMapGenerator::DrawDebugPoisonDisk()
{
	DrawDebugGrid();
	DrawDebugStartEndPoints();
	
	UWorld* World = GetWorld();

	for (int Index = 0; Index < GeneratedPoints.Num(); ++Index)
	{
		FVector Position = FVector(GeneratedPoints[Index].X, GeneratedPoints[Index].Y, 0.0f);
		DrawDebugSphere(World, Position, SphereRadius, 10, FColor::Red);
		DrawDebugSphere(World, Position, 0.1, 10, FColor::Blue);
	}

}


// Called every frame
void AMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DrawDebugPoisonDisk();
}
