// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/Modifiers/VampiricModifier.h"
#include "Characters/BaseCharacter.h"

void UVampiricModifier::OnHitEffect(ABaseCharacter* Hitter, ABaseCharacter* HitTarget)
{
	Super::OnHitEffect(Hitter, HitTarget);

	if (Hitter && HitTarget)
	{
		Hitter->VampiricHeal(HealAmount);
		HitTarget->VampiricHit();
	}
}
