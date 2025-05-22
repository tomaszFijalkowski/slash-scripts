// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoliageManager.generated.h"

class UFoliageInstancedStaticMeshComponent;
class ABreakableActor;

UCLASS()
class SLASH_API AFoliageManager : public AActor
{
	GENERATED_BODY()

public:
	AFoliageManager();
	void HandleCloseFoliage(const FHitResult& HitResult);
	void HandleFarFoliage(AActor* OtherActor);

private:
	void HandleCloseFoliageInstancedMesh(UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh,
	                                     const int32 FoliageItemNumber);
	void HandleCloseBreakableFoliage(const ABreakableActor* BreakableFoliage);
	void ScheduleBreakableFoliageDespawn(ABreakableActor* BreakableFoliage);

	TSubclassOf<ABreakableActor> GetBreakableFoliageToSpawn(
		const UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh);
	void SpawnBreakableFoliage(UFoliageInstancedStaticMeshComponent* FoliageInstancedMesh,
	                           const FTransform FoliageInstanceTransform,
	                           const TSubclassOf<ABreakableActor>& BreakableFoliageBlueprintToSpawn);
	void RemoveBreakableFoliageDespawnTimer(FGuid FoliageIdentifier);
	void SpawnFoliageInstance(ABreakableActor* BreakableFoliage);

	UPROPERTY(EditAnywhere, Category = "Foliage Properties")
	TMap<UStaticMesh*, TSubclassOf<ABreakableActor>> BreakableFoliageClasses;

	UPROPERTY(EditAnywhere, Category = "Foliage Properties")
	float BreakableFoliageDespawnDelay = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Foliage Properties")
	TMap<FGuid, FTimerHandle> BreakableFoliageDespawnTimers;
};
