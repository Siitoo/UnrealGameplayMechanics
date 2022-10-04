// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeneratedEdge.generated.h"
/**
 * 
 */
USTRUCT()
struct FGeneratedEdge
{
	GENERATED_BODY()

	FGeneratedEdge();
	FGeneratedEdge(FVector2D &v1, FVector2D &v2);

public:
	FVector2D StartPoint, EndPoint;
	bool bIsBad = false;

};