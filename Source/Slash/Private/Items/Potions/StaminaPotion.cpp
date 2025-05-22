// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/StaminaPotion.h"
#include "Interfaces/PickupInterface.h"

void AStaminaPotion::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                          const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		if (PickupInterface->CanUseStaminaPotion())
		{
			PickupInterface->PickupStaminaPotion(this);
			SpawnPickupEffect();
			PlayPickupSound();
			FinishEmissionAndDestroy();
		}
	}
}
