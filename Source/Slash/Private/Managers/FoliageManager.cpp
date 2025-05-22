// Fill out your copyright notice in the Description page of Project Settings.

#include "Managers/FoliageManager.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "../../Public/Breakables/BreakableActor.h"

AFoliageManager::AFoliageManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFoliageManager::HandleCloseFoliage(const FHitResult& HitResult)
{
	if (UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh =
		Cast<UFoliageInstancedStaticMeshComponent>(HitResult.GetComponent()))
	{
		HandleCloseFoliageInstancedMesh(FoliageInstancedMesh, HitResult.Item);
	}
	else if (const ABreakableActor* BreakableFoliage = Cast<ABreakableActor>(HitResult.GetActor()))
	{
		HandleCloseBreakableFoliage(BreakableFoliage);
	}
}

void AFoliageManager::HandleFarFoliage(AActor* OtherActor)
{
	ABreakableActor* BreakableFoliage = Cast<ABreakableActor>(OtherActor);
	if (BreakableFoliage && !BreakableFoliage->IsBroken())
	{
		const FGuid FoliageIdentifier = BreakableFoliage->GetFoliageIdentifier();
		if (!BreakableFoliageDespawnTimers.Contains(FoliageIdentifier))
		{
			ScheduleBreakableFoliageDespawn(BreakableFoliage);
		}
	}
}

void AFoliageManager::HandleCloseFoliageInstancedMesh(UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh,
                                                      const int32 FoliageItemNumber)
{
	check(FoliageInstancedMesh);

	const TSubclassOf<ABreakableActor> BreakableFoliageToSpawn = GetBreakableFoliageToSpawn(FoliageInstancedMesh);
	if (BreakableFoliageToSpawn)
	{
		FTransform FoliageInstanceTransform;
		FoliageInstancedMesh->GetInstanceTransform(FoliageItemNumber, FoliageInstanceTransform, true);
		FoliageInstancedMesh->RemoveInstance(FoliageItemNumber);

		SpawnBreakableFoliage(FoliageInstancedMesh, FoliageInstanceTransform, BreakableFoliageToSpawn);
	}
}

void AFoliageManager::HandleCloseBreakableFoliage(const ABreakableActor* BreakableFoliage)
{
	check(BreakableFoliage);

	const FGuid FoliageIdentifier = BreakableFoliage->GetFoliageIdentifier();
	if (FoliageIdentifier.IsValid())
	{
		RemoveBreakableFoliageDespawnTimer(FoliageIdentifier);
	}
}

void AFoliageManager::ScheduleBreakableFoliageDespawn(ABreakableActor* BreakableFoliage)
{
	check(BreakableFoliage);

	const FGuid FoliageIdentifier = BreakableFoliage->GetFoliageIdentifier();

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this, BreakableFoliage]
	{
		SpawnFoliageInstance(BreakableFoliage);
	});

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, BreakableFoliageDespawnDelay, false);

	BreakableFoliageDespawnTimers.Add(FoliageIdentifier, TimerHandle);
}

TSubclassOf<ABreakableActor> AFoliageManager::GetBreakableFoliageToSpawn(
	const UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh)
{
	check(FoliageInstancedMesh);

	const UStaticMesh* FoliageInstanceMesh = FoliageInstancedMesh->GetStaticMesh();
	if (FoliageInstanceMesh)
	{
		const TSubclassOf<ABreakableActor>* BreakableFoliageToSpawn = BreakableFoliageClasses.Find(FoliageInstanceMesh);
		return BreakableFoliageToSpawn ? *BreakableFoliageToSpawn : nullptr;
	}

	return nullptr;
}

void AFoliageManager::SpawnBreakableFoliage(UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh,
                                            const FTransform FoliageInstanceTransform,
                                            const TSubclassOf<ABreakableActor>& BreakableFoliageBlueprintToSpawn)
{
	check(FoliageInstancedMesh);
	check(BreakableFoliageBlueprintToSpawn);

	if (UWorld* World = GetWorld())
	{
		ABreakableActor* SpawnedActor = World->SpawnActor<ABreakableActor>(
			BreakableFoliageBlueprintToSpawn, FoliageInstanceTransform);

		if (SpawnedActor)
		{
			const FGuid FoliageIdentifier = FGuid::NewGuid();
			SpawnedActor->SetFoliageProperties(FoliageInstancedMesh, FoliageInstanceTransform, FoliageIdentifier);
		}
	}
}

void AFoliageManager::RemoveBreakableFoliageDespawnTimer(const FGuid FoliageIdentifier)
{
	if (BreakableFoliageDespawnTimers.Contains(FoliageIdentifier))
	{
		FTimerHandle TimerHandle = BreakableFoliageDespawnTimers[FoliageIdentifier];
		GetWorldTimerManager().ClearTimer(TimerHandle);
		BreakableFoliageDespawnTimers.Remove(FoliageIdentifier);
	}
}

void AFoliageManager::SpawnFoliageInstance(ABreakableActor* BreakableFoliage)
{
	check(BreakableFoliage);

	UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh = BreakableFoliage->GetFoliageInstancedMesh();
	if (FoliageInstancedMesh)
	{
		const FGuid FoliageIdentifier = BreakableFoliage->GetFoliageIdentifier();
		BreakableFoliageDespawnTimers.Remove(FoliageIdentifier);

		BreakableFoliage->Destroy();

		const FTransform FoliageTransform = BreakableFoliage->GetFoliageTransform();
		FoliageInstancedMesh->AddInstance(FoliageTransform, true);
	}
}
