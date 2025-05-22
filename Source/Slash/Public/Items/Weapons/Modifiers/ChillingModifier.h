// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponModifier.h"
#include "ChillingModifier.generated.h"

class UNiagaraComponent;

UCLASS(Blueprintable, BlueprintType)
class SLASH_API UChillingModifier : public UWeaponModifier
{
	GENERATED_BODY()

protected:
	virtual void OnHitEffect(ABaseCharacter* Hitter, ABaseCharacter* HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Modifier Properties")
	float SlowMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, Category = "Modifier Properties")
	float Duration = 4.f;
};
