// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CharacterTargetPoint.h"

void ACharacterTargetPoint::MarkBothAsOccupied()
{
	MarkAsOccupied();
	if (LinkedTargetPoint)
	{
		LinkedTargetPoint->MarkAsOccupied();
	}
}

void ACharacterTargetPoint::MarkBothAsUnoccupied()
{
	MarkAsUnoccupied();
	if (LinkedTargetPoint)
	{
		LinkedTargetPoint->MarkAsUnoccupied();
	}
}
