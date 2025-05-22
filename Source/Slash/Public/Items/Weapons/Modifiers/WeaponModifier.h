// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponModifier.generated.h"

class ABaseCharacter;

UCLASS(Blueprintable, BlueprintType)
class SLASH_API UWeaponModifier : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnHitEffect(ABaseCharacter* Hitter, ABaseCharacter* HitTarget);
};
