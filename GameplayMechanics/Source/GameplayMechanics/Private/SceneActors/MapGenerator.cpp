// Fill out your copyright notice in the Description page of Project Settings.

#include "SceneActors/MapGenerator.h"
#include "DrawDebugHelpers.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AMapGenerator::AMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//StartPoint = FVector(0.0f);
	//EndPoint = FVector(50.0f, 0.0f, 0.0f);

	Seed = 123456789;
	GridExtend = 50.0f;
	SphereRadius = 5.0f;
	Iterations = 50000;
	NumSampleBeforeRejection = 1;
	bCheckWellGenerated = false;

	
	bDebugGrid = true;
	bDebugPoisonDisk = false;
	bDebugDelaunary = false;
	
	PathfindingIterations = 1;
}

// Called when the game starts or when spawned
void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	

	//MyPoisonDiskSamplingAlgorithm();
	
	//DelaunaryTriangulation();
	//GeneratePaths();
	//DrawDebugPathGenerated();
}

void AMapGenerator::MyPoisonDiskSamplingAlgorithm()
{
	Random = FRandomStream(Seed);

	const float PISimplified = 3.141592654f;

	const FVector2D RegionSize = FVector2D(GridExtend, GridExtend);
	const float CellSize = SphereRadius / FMath::Sqrt(2.0f);
	const int MaxGridCellsX = ceil(GridExtend / CellSize);
	const int MaxGridCellsY = MaxGridCellsX;
	const int GridSize = MaxGridCellsX * MaxGridCellsY;

	GeneratedPoints.Reset(0);
	Grid.Reset(0);
	Grid.SetNumZeroed(GridSize);

	TArray<FVector2D> SpawnPoints;

	//StartedPoint, we try with the middle point
	FVector2D StartedPoint = FVector2D(GridExtend / 2.0f, GridExtend / 2.0f);
	SpawnPoints.Add(StartedPoint);

	if (Iterations < 0 || Iterations >= MAX_int32)
	{
		Iterations = MAX_int32;
	}

	while (SpawnPoints.Num() > 0 && Iterations > 0)
	{
		const int RandomSpawnIndex = Random.RandRange(0, SpawnPoints.Num() - 1);
		const FVector2D SpawnRandomPoint = SpawnPoints[RandomSpawnIndex];

		if (Iterations == 1)
		{
			int x = 1;
		}

		bool bCandidateAccepted = false;

		for (int Index = 0; Index < NumSampleBeforeRejection; ++Index)
		{
			const float Angle = Random.FRand() * PISimplified * 2;
			const FVector2D Direction = FVector2D(FMath::Sin(Angle), FMath::Cos(Angle));
			const FVector2D CandidatePoint = SpawnRandomPoint + Direction * Random.RandRange(SphereRadius, SphereRadius * 2.0f);

			if (IsMyCandidateValid(CandidatePoint, RegionSize, CellSize, Grid))
			{
				GeneratedPoints.Add(CandidatePoint);
				SpawnPoints.Add(CandidatePoint);

				const int LocationX = (int)(CandidatePoint.X / CellSize);
				const int LocationY = (int)(CandidatePoint.Y / CellSize);

				const int GridIndex = MaxGridCellsX * LocationX + LocationY;
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

	StartPoint = FVector2D(-10.0f, GridExtend/2.0f);
	EndPoint = FVector2D(GridExtend + 10.0f, GridExtend / 2.0f);

	GeneratedPoints.Add(StartPoint);
	GeneratedPoints.Add(EndPoint);

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

bool AMapGenerator::IsMyCandidateValid(FVector2D Candidate, FVector2D RegionSize, float CellSize, TArray<int> GridCells)
{
	if (Candidate.X >= 0 && Candidate.X < RegionSize.X && Candidate.Y >= 0 && Candidate.Y < RegionSize.Y)
	{
		int LocationX = (int)(Candidate.X / CellSize);
		int LocationY = (int)(Candidate.Y / CellSize);

		//To search 5 by 5 around the cell
		int SearchStartX = FMath::Max(0, LocationX - 2);
		int SearchEndX = FMath::Min(LocationX + 2, RegionSize.X / CellSize);

		int SearchStartY = FMath::Max(0, LocationY - 2);
		int SearchEndY = FMath::Min(LocationY + 2, RegionSize.Y/ CellSize);

		for (int X = SearchStartX; X <= SearchEndX; ++X)
		{
			for (int Y = SearchStartY; Y <= SearchEndY; ++Y)
			{
				int Index = ceil(RegionSize.X/CellSize) * X + Y;

				if (Index >= GridCells.Num())
				{
					return false;
				}

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

void AMapGenerator::DelaunaryTriangulation()
{
	if (GeneratedPoints.Num() > 0)
	{
		Triangles.Reset(0);
		Edges.Reset(0);

		FVector2D MinVertices = GeneratedPoints[0];
		FVector2D MaxVertices = MinVertices;

		//Generating the Super Triangle
		FVector2D Vertice;
		for (int Index = 1; Index < GeneratedPoints.Num(); ++Index)
		{
			Vertice = GeneratedPoints[Index];

			if (Vertice.X > MaxVertices.X) MaxVertices.X = Vertice.X;
			if (Vertice.Y > MaxVertices.Y) MaxVertices.Y = Vertice.Y;
			if (Vertice.X < MinVertices.X) MinVertices.X = Vertice.X;
			if (Vertice.X < MinVertices.Y) MinVertices.Y = Vertice.X;
		}

		const float Dx = MaxVertices.X - MinVertices.X;
		const float Dy = MaxVertices.Y - MinVertices.Y;

		const float DeltaMax = FMath::Max(Dx, Dy);

		const float MidX = (MaxVertices.X + MinVertices.X) / 2.0f;
		const float MidY = (MaxVertices.Y + MinVertices.Y) / 2.0f;

		FVector2D LeftVertex = FVector2D(MidX - 50.0f * DeltaMax, MidY - DeltaMax);
		FVector2D RightVertex = FVector2D(MidX + 50.0f * DeltaMax, MidY - DeltaMax);
		FVector2D UpVertex = FVector2D(MidX, MidY + 50.0f * DeltaMax);

		FGeneratedTriangle SuperTriangle = FGeneratedTriangle(LeftVertex, RightVertex, UpVertex);
		
		Triangles.Add(SuperTriangle);

		for (int Index = 0; Index < GeneratedPoints.Num(); ++Index)
		{
			TArray<FGeneratedEdge> Polygons;
			
			for (int TriangleIndex = 0; TriangleIndex < Triangles.Num(); ++TriangleIndex)
			{
				if (Triangles[TriangleIndex].CircumCircleContains(GeneratedPoints[Index]))
				{
					Triangles[TriangleIndex].bIsBad = true;

					Polygons.Add(FGeneratedEdge(Triangles[TriangleIndex].Vertex1, Triangles[TriangleIndex].Vertex2));
					Polygons.Add(FGeneratedEdge(Triangles[TriangleIndex].Vertex2, Triangles[TriangleIndex].Vertex3));
					Polygons.Add(FGeneratedEdge(Triangles[TriangleIndex].Vertex3, Triangles[TriangleIndex].Vertex1));
				}
			}

			for (int TriangleIndex = 0; TriangleIndex < Triangles.Num(); ++TriangleIndex) //Can change to lamda function
			{
				if (Triangles[TriangleIndex].bIsBad)
				{
					Triangles.RemoveAt(TriangleIndex);
					TriangleIndex -= 1;
				}
			}

			for (int PolygonIndex = 0; PolygonIndex < Polygons.Num(); ++PolygonIndex)
			{
				for (int NextPolygonIndex = PolygonIndex + 1; NextPolygonIndex < Polygons.Num(); ++NextPolygonIndex)
				{
					FGeneratedEdge A = Polygons[PolygonIndex];
					FGeneratedEdge B = Polygons[NextPolygonIndex];

					if ((FMath::IsNearlyEqual(A.StartPoint.X,B.StartPoint.X) && FMath::IsNearlyEqual(A.StartPoint.Y, B.StartPoint.Y) &&
						FMath::IsNearlyEqual(A.EndPoint.X, B.EndPoint.X) && FMath::IsNearlyEqual(A.EndPoint.Y, B.EndPoint.Y) )|| 
						(FMath::IsNearlyEqual(A.StartPoint.X, B.EndPoint.X) && FMath::IsNearlyEqual(A.StartPoint.Y, B.EndPoint.Y) &&
						FMath::IsNearlyEqual(A.EndPoint.X, B.StartPoint.X) && FMath::IsNearlyEqual(A.EndPoint.Y, B.StartPoint.Y))) 
					{
						Polygons[PolygonIndex].bIsBad = true;
						Polygons[NextPolygonIndex].bIsBad = true;
					}
				}
			}

			for (int PolygonIndex = 0; PolygonIndex < Polygons.Num(); ++PolygonIndex)
			{
				if (Polygons[PolygonIndex].bIsBad)
				{
					Polygons.RemoveAt(PolygonIndex);
					PolygonIndex -= 1;
				}
			}

			for (int PolygonIndex = 0; PolygonIndex < Polygons.Num(); ++PolygonIndex)
			{
				Triangles.Add(FGeneratedTriangle(Polygons[PolygonIndex].StartPoint, Polygons[PolygonIndex].EndPoint, GeneratedPoints[Index]));
			}
		}

		//Remove Triangles that have conections with the super triangle
		for (int TriangleIndex = 0; TriangleIndex < Triangles.Num(); ++TriangleIndex)
		{
			FGeneratedTriangle Triangle = Triangles[TriangleIndex];

			bool bContainSuperTriangle = false;

			if (Triangle.Vertex1 == LeftVertex || Triangle.Vertex2 == LeftVertex || Triangle.Vertex3 == LeftVertex)
			{
				bContainSuperTriangle = true;
			}
			else if (Triangle.Vertex1 == RightVertex || Triangle.Vertex2 == RightVertex || Triangle.Vertex3 == RightVertex)
			{
				bContainSuperTriangle = true;
			}
			else if (Triangle.Vertex1 == UpVertex || Triangle.Vertex2 == UpVertex || Triangle.Vertex3 == UpVertex)
			{
				bContainSuperTriangle = true;
			}

			if (bContainSuperTriangle)
			{
				Triangles.RemoveAt(TriangleIndex);
				TriangleIndex -= 1;
			}
		}

		for (int TriangleIndex = 0; TriangleIndex < Triangles.Num(); ++TriangleIndex)
		{
			Edges.Add(FGeneratedEdge(Triangles[TriangleIndex].Vertex1, Triangles[TriangleIndex].Vertex2));
			Edges.Add(FGeneratedEdge(Triangles[TriangleIndex].Vertex2, Triangles[TriangleIndex].Vertex3));
			Edges.Add(FGeneratedEdge(Triangles[TriangleIndex].Vertex3, Triangles[TriangleIndex].Vertex1));
		}
	}



}

void AMapGenerator::GeneratePaths()
{
	Paths.Reset(0);

	//Pre generate the points on Path
	FGeneratedNode StartingPointPath;
	StartingPointPath.NodePosition = StartPoint;
	Paths.Add(StartingPointPath);

	for (int Index = 0; Index < GeneratedPoints.Num()-2; ++Index)
	{
		FGeneratedNode PathNode;
		PathNode.NodePosition = GeneratedPoints[Index];
		int PathIndex = Paths.Add(PathNode);
	}

	FGeneratedNode EndPointPath;
	EndPointPath.NodePosition = EndPoint;
	Paths.Add(EndPointPath);


	for (int Index = 0; Index < Paths.Num(); ++Index)
	{
		FVector2D* NodePosition = &Paths[Index].NodePosition;

		for (auto EdgeIt = Edges.CreateConstIterator(); EdgeIt; ++EdgeIt)
		{
			FVector2D EdgePosition = FVector2D(-1.0f);

			if (EdgeIt->StartPoint.Equals(*NodePosition))
			{
				EdgePosition = EdgeIt->EndPoint;
			}
			else if (EdgeIt->EndPoint.Equals(*NodePosition))
			{
				EdgePosition = EdgeIt->StartPoint;
			}

			if (EdgePosition.X != -1)
			{
				if ((EdgePosition.X - NodePosition->X) > SphereRadius / 3.0f)
				{
					FGeneratedNode* TrackedNode = nullptr;
					for (auto It = Paths.CreateConstIterator(); It; ++It)
					{
						if (It->NodePosition.Equals(EdgePosition))
						{
							TrackedNode = &Paths[It.GetIndex()];
							break;
						}
					}

					if (TrackedNode != nullptr)
					{
						Paths[Index].ChildNodes.Add(TrackedNode);
						TrackedNode->ParentNode = &Paths[Index];
					}
				}
			}

		}

	}
}


void AMapGenerator::DrawDebugGrid()
{
	UWorld* World = GetWorld();

	const FVector BoxCenter = FVector(GridExtend / 2.0f, GridExtend / 2.0f, 0.1f);
	const float CellSize = SphereRadius / FMath::Sqrt(2.0f);
	const int MaxGridCells = ceil(GridExtend / CellSize);

	DrawDebugBox(World, BoxCenter, BoxCenter, FColor::Green);

	FVector Start = FVector::Zero();
	FVector End = FVector::Zero();
	End.X = GridExtend;

	for (int Index = 0; Index < MaxGridCells; ++Index)
	{
		Start.Y = Index * CellSize;
		End.Y = Index * CellSize;

		DrawDebugLine(World, Start, End, FColor::Black);
	}

	Start = FVector::Zero();
	End = FVector::Zero();
	End.Y = GridExtend;

	for (int Index = 0; Index < MaxGridCells; ++Index)
	{
		Start.X = Index * CellSize;
		End.X = Index * CellSize;

		DrawDebugLine(World, Start, End, FColor::Black);
	}
}

void AMapGenerator::DrawDebugPoisonDisk()
{
	UWorld* World = GetWorld();

	for (int Index = 0; Index < GeneratedPoints.Num(); ++Index)
	{
		const FVector Position = FVector(GeneratedPoints[Index].X, GeneratedPoints[Index].Y, 0.0f);
		DrawDebugSphere(World, Position, 0.1, 4, FColor::Green);
	}
}

void AMapGenerator::DrawDebugDelaunary()
{
	UWorld* World = GetWorld();

	for (int Index = 0; Index < Triangles.Num(); ++Index)
	{
		FVector Vert1 = FVector(Triangles[Index].Vertex1.X, Triangles[Index].Vertex1.Y, 0.0f);
		FVector Vert2 = FVector(Triangles[Index].Vertex2.X, Triangles[Index].Vertex2.Y, 0.0f);
		FVector Vert3 = FVector(Triangles[Index].Vertex3.X, Triangles[Index].Vertex3.Y, 0.0f);

		DrawDebugLine(World, Vert1, Vert2, FColor::Blue);
		DrawDebugLine(World, Vert2, Vert3, FColor::Blue);
		DrawDebugLine(World, Vert3, Vert1, FColor::Blue);
	}

	for (int Index = 0; Index < Edges.Num(); ++Index)
	{
		FVector Vert1 = FVector(Edges[Index].StartPoint.X, Edges[Index].StartPoint.Y, 0.0f);
		FVector Vert2 = FVector(Edges[Index].EndPoint.X, Edges[Index].EndPoint.Y, 0.0f);

		DrawDebugLine(World, Vert1, Vert2, FColor::Blue);
	}
}

void AMapGenerator::DrawDebugPathGenerated()
{
	UWorld* World = GetWorld();

	UKismetSystemLibrary::FlushPersistentDebugLines(World);

	for (int Index = 0; Index < Paths.Num(); ++Index)
	{
		int ColorValueR = Random.RandRange(0, 255);
		int ColorValueG = Random.RandRange(0, 255);
		int ColorValueB = Random.RandRange(0, 255);

		FColor RandomColor = FColor(ColorValueR, ColorValueG, ColorValueB);

		FVector PathPoint = FVector(Paths[Index].NodePosition.X, Paths[Index].NodePosition.Y, 0.0f);

		DrawDebugSphere(World, PathPoint, 0.1f, 10, RandomColor, true);

		for (int LinkedIndex = 0; LinkedIndex < Paths[Index].ChildNodes.Num(); ++LinkedIndex)
		{
			FVector NexPathPoint = FVector(Paths[Index].ChildNodes[LinkedIndex]->NodePosition.X, Paths[Index].ChildNodes[LinkedIndex]->NodePosition.Y, 0.0f);

			DrawDebugLine(World, PathPoint, NexPathPoint, RandomColor, true);
		}

	}
}

// Called every frame
void AMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugGrid)
	{
		DrawDebugGrid();
	}
	
	if (bDebugPoisonDisk)
	{
		DrawDebugPoisonDisk();
	}
	
	if (bDebugDelaunary)
	{
		DrawDebugDelaunary();
	}

	if (bDebugGeneratedPath)
	{
		DrawDebugPathGenerated();
		bDebugGeneratedPath = false;
	}


}


