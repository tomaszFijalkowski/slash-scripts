// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/Modifiers/PoisonousModifier.h"
#include "Characters/BaseCharacter.h"

void UPoisonousModifier::OnHitEffect(ABaseCharacter* Hitter, ABaseCharacter* HitTarget)
{
	Super::OnHitEffect(Hitter, HitTarget);

	if (HitTarget)
	{
		HitTarget->ApplyPoison(Damage, Duration);
	}
}
