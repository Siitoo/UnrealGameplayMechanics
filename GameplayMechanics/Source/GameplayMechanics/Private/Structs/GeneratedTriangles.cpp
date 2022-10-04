// Fill out your copyright notice in the Description page of Project Settings.
#include "Structs/GeneratedTriangles.h"

FGeneratedTriangle::FGeneratedTriangle()
{
}

FGeneratedTriangle::FGeneratedTriangle(FVector2D& v1, FVector2D& v2, FVector2D& v3) :
	Vertex1(v1), Vertex2(v2), Vertex3(v3)
{
};

bool FGeneratedTriangle::CircumCircleContains(const FVector2D &Vertex) const
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