// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponModifier.h"
#include "PoisonousModifier.generated.h"

class UNiagaraComponent;

UCLASS(Blueprintable, BlueprintType)
class SLASH_API UPoisonousModifier : public UWeaponModifier
{
	GENERATED_BODY()

protected:
	virtual void OnHitEffect(ABaseCharacter* Hitter, ABaseCharacter* HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Modifier Properties")
	float Damage = 15.f;

	UPROPERTY(EditAnywhere, Category = "Modifier Properties")
	float Duration = 3.f;
};
