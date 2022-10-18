// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeneratedNode.generated.h"
/**
 * 
 */
USTRUCT()
struct FGeneratedNode
{
	GENERATED_BODY()

	FGeneratedNode();


public:
	FVector2D NodePosition;
	TArray<FGeneratedNode*> ChildNodes;
	FGeneratedNode* ParentNode;
	int Score = 0;
};