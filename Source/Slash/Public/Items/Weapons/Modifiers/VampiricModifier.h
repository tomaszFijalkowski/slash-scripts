// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponModifier.h"
#include "VampiricModifier.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SLASH_API UVampiricModifier : public UWeaponModifier
{
	GENERATED_BODY()

public:
	virtual void OnHitEffect(ABaseCharacter* Hitter, ABaseCharacter* HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Modifier Properties")
	float HealAmount = 6.f;
};
