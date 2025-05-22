// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/Modifiers/ChillingModifier.h"
#include "Characters/BaseCharacter.h"

void UChillingModifier::OnHitEffect(ABaseCharacter* Hitter, ABaseCharacter* HitTarget)
{
	Super::OnHitEffect(Hitter, HitTarget);

	if (HitTarget)
	{
		HitTarget->ApplyChill(SlowMultiplier, Duration);
	}
}
