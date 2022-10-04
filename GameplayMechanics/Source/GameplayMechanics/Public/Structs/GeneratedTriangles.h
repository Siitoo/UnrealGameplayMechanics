// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeneratedTriangles.generated.h"
/**
 * 
 */
USTRUCT()
struct FGeneratedTriangle
{
	GENERATED_BODY()

	FGeneratedTriangle();
	FGeneratedTriangle(FVector2D& v1, FVector2D& v2, FVector2D& v3);

	bool CircumCircleContains(const FVector2D &v) const;

public:
	FVector2D Vertex1, Vertex2, Vertex3;
	bool bIsBad = false;
};