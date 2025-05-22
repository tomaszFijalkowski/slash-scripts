// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "CharacterTargetPoint.generated.h"

UCLASS()
class SLASH_API ACharacterTargetPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	void MarkBothAsOccupied();
	void MarkBothAsUnoccupied();

private:
	bool bIsOccupied = false;
	TObjectPtr<ACharacterTargetPoint> LinkedTargetPoint;

public:
	FORCEINLINE bool IsOccupied() const { return bIsOccupied; }
	FORCEINLINE void MarkAsOccupied() { bIsOccupied = true; }
	FORCEINLINE void MarkAsUnoccupied() { bIsOccupied = false; }
	FORCEINLINE TObjectPtr<ACharacterTargetPoint> GetLinkedTargetPoint() { return LinkedTargetPoint; }
	FORCEINLINE void SetLinkedTargetPoint(const TObjectPtr<ACharacterTargetPoint>& TargetPoint)
	{
		LinkedTargetPoint = TargetPoint;
	}
};
