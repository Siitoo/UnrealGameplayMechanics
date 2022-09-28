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
	DelaunaryTriangulation();
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

void AMapGenerator::DelaunaryTriangulation()
{
	if (GeneratedPoints.Num() > 0)
	{
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

		const FVector2D LeftVertex = FVector2D(MidX - 50.0f * DeltaMax, MidY - DeltaMax);
		const FVector2D RightVertex = FVector2D(MidX + 50.0f * DeltaMax, MidY - DeltaMax);
		const FVector2D UpVertex = FVector2D(MidX, MidY + 50.0f * DeltaMax);

		SuperTriangle = FGeneratedTriangle(LeftVertex, RightVertex, UpVertex);
		
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
	//DrawDebugGrid();
	//DrawDebugStartEndPoints();
	
	UWorld* World = GetWorld();

	for (int Index = 0; Index < GeneratedPoints.Num(); ++Index)
	{
		FVector Position = FVector(GeneratedPoints[Index].X, GeneratedPoints[Index].Y, 0.0f);
		//DrawDebugSphere(World, Position, SphereRadius, 10, FColor::Red);
		DrawDebugSphere(World, Position, 0.1, 10, FColor::Blue);
	}

}

void AMapGenerator::DrawDebugDelaunary()
{
	UWorld* World = GetWorld();

	FVector V1 = FVector(SuperTriangle.Vertex1.X, SuperTriangle.Vertex1.Y,0.0f);
	FVector V2 = FVector(SuperTriangle.Vertex2.X, SuperTriangle.Vertex2.Y, 0.0f);
	FVector V3 = FVector(SuperTriangle.Vertex3.X, SuperTriangle.Vertex3.Y, 0.0f);

	//DrawDebugLine(World, V1, V2, FColor::Orange);
	//DrawDebugLine(World, V2, V3, FColor::Orange);
	//DrawDebugLine(World, V3, V1, FColor::Orange);


	/*for (int Index = 0; Index < Triangles.Num(); ++Index)
	{
		FVector Vert1 = FVector(Triangles[Index].Vertex1.X, Triangles[Index].Vertex1.Y, 0.0f);
		FVector Vert2 = FVector(Triangles[Index].Vertex2.X, Triangles[Index].Vertex2.Y, 0.0f);
		FVector Vert3 = FVector(Triangles[Index].Vertex3.X, Triangles[Index].Vertex3.Y, 0.0f);

		DrawDebugLine(World, Vert1, Vert2, FColor::Blue);
		DrawDebugLine(World, Vert2, Vert3, FColor::Blue);
		DrawDebugLine(World, Vert3, Vert1, FColor::Blue);
	}*/

	for (int Index = 0; Index < Edges.Num(); ++Index)
	{
		FVector Vert1 = FVector(Edges[Index].StartPoint.X, Edges[Index].StartPoint.Y, 0.0f);
		FVector Vert2 = FVector(Edges[Index].EndPoint.X, Edges[Index].EndPoint.Y, 0.0f);

		DrawDebugLine(World, Vert1, Vert2, FColor::Blue);
	}
}


// Called every frame
void AMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugPoisonDisk();
	DrawDebugDelaunary();
}

//https://github.com/bl4ckb0ne/delaunay-triangulation/tree/master/dt

bool FGeneratedTriangle::CircumCircleContains(const FVector2D Vertex) const
{
	const float Ab = Vertex1.SizeSquared();
	const float Cd = Vertex2.SizeSquared();
	const float Ef = Vertex3.SizeSquared();

	const float Ax = Vertex1.X;
	const float Ay = Vertex1.Y;
	const float Bx = Vertex2.X;
	const float By = Vertex2.Y;
	const float Cx = Vertex3.X;
	const float Cy = Vertex3.Y;

	const float Circum_X = (Ab * (Cy - By) + Cd * (Ay - Cy) + Ef * (By - Ay)) / (Ax * (Cy - By) + Bx * (Ay - Cy) + Cx * (By - Ay));
	const float Circum_Y = (Ab * (Cx - Bx) + Cd * (Ax - Cx) + Ef * (Bx - Ax)) / (Ay * (Cx - Bx) + By * (Ax - Cx) + Cy * (Bx - Ax));

	const FVector2D Circum = FVector2D(Circum_X/2.0f, Circum_Y/2.0f);

	const float DxV1 = Ax - Circum.X;
	const float DyV1 = Ay - Circum.Y;
	const float RaidusCircum = DxV1 * DxV1 + DyV1 * DyV1;

	const float DxVertex = Vertex.X - Circum.X;
	const float DyVertex = Vertex.Y - Circum.Y;
	const float Distance = DxVertex * DxVertex + DyVertex * DyVertex;

	return Distance <= RaidusCircum;
}
