// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/HealthPotion.h"
#include "Interfaces/PickupInterface.h"

void AHealthPotion::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		if (PickupInterface->CanUseHealthPotion())
		{
			PickupInterface->PickupHealthPotion(this);
			SpawnPickupEffect();
			PlayPickupSound();
			FinishEmissionAndDestroy();
		}
	}
}
