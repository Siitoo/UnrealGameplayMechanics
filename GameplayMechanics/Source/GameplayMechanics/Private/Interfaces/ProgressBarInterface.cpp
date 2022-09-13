// Fill out your copyright notice in the Description page of Project Settings.


#include "Interfaces/ProgressBarInterface.h"

// Add default functionality here for any IProgressBarInterface functions that are not pure virtual.

float IProgressBarInterface::GetCurrentHealth()
{
	return 0.0f;
}

float IProgressBarInterface::GetHealthAsRatio()
{
	return 0.0f;
}

float IProgressBarInterface::GetCurrentMana()
{
	return 0.0f;
}

float IProgressBarInterface::GetManaAsRatio()
{
	return 0.0f;
}
