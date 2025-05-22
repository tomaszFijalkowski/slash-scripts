// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "HealthPotion.generated.h"

UCLASS()
class SLASH_API AHealthPotion : public AItem
{
	GENERATED_BODY()

protected:
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                  const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Health Potion Properties")
	float HealthAmount = 30.f;

	UPROPERTY(EditAnywhere, Category = "Health Potion Properties")
	float HealthRegenDuration = 5.f;

	UPROPERTY(EditAnywhere, Category = "Health Potion Properties")
	float HealthRegenRateMultiplier = 6.f;

public:
	FORCEINLINE float GetHealthAmount() const { return HealthAmount; }
	FORCEINLINE float GetHealthRegenDuration() const { return HealthRegenDuration; }
	FORCEINLINE float GetHealthRateMultiplier() const { return HealthRegenRateMultiplier; }
};
