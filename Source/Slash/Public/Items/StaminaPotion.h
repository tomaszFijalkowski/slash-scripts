// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "StaminaPotion.generated.h"

UCLASS()
class SLASH_API AStaminaPotion : public AItem
{
	GENERATED_BODY()

protected:
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                  const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Stamina Potion Properties")
	float StaminaAmount = 30.f;

	UPROPERTY(EditAnywhere, Category = "Stamina Potion Properties")
	float StaminaRegenDuration = 10.f;

	UPROPERTY(EditAnywhere, Category = "Stamina Potion Properties")
	float StaminaRegenRateMultiplier = 2.f;

	UPROPERTY(EditAnywhere, Category = "Stamina Potion Properties")
	float WalkSpeedMultiplier = 1.1f;

	UPROPERTY(EditAnywhere, Category = "Stamina Potion Properties")
	float AnimationRateScale = 1.2f;

public:
	FORCEINLINE float GetStaminaAmount() const { return StaminaAmount; }
	FORCEINLINE float GetStaminaRegenDuration() const { return StaminaRegenDuration; }
	FORCEINLINE float GetStaminaRateMultiplier() const { return StaminaRegenRateMultiplier; }
	FORCEINLINE float GetWalkSpeedMultiplier() const { return WalkSpeedMultiplier; }
	FORCEINLINE float GetAnimationRateScale() const { return AnimationRateScale; }
};
