// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemySpawnPoint;
class AEnemy;
class ATargetPoint;
class ASlashCharacter;
class USphereComponent;

UCLASS()
class SLASH_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                          const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void OnInitialSphereOverlap();
	void HandleOverlappingCharacter();
	void HandleShowingEnemies(float DistanceToCharacter, float SphereRadius);
	void HandleSpawningEnemies(float DistanceToCharacter, float SphereRadius);
	void HandleActivatingEnemies(float DistanceToCharacter, float SphereRadius);

	void ShowEnemies();
	void HideEnemies();

	void SpawnEnemies();
	void DespawnEnemies();

	void UpdateAlliedEnemies();

	void ActivateEnemies();
	void DeactivateEnemies();

	UPROPERTY(EditInstanceOnly, Category = "Spawner Properties")
	TArray<TObjectPtr<AEnemySpawnPoint>> SpawnPoints;

	UPROPERTY(VisibleInstanceOnly, Category = "Spawner Properties")
	TArray<TObjectPtr<AEnemy>> SpawnedEnemies;

	UPROPERTY(VisibleAnywhere, Category = "Spawner Properties")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(EditAnywhere, Category = "Spawner Properties")
	float ShowRadiusFraction = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Spawner Properties")
	float HideRadiusFraction = 1.f;

	UPROPERTY(EditAnywhere, Category = "Spawner Properties")
	float SpawnRadiusFraction = 0.45f;

	UPROPERTY(EditAnywhere, Category = "Spawner Properties")
	float DespawnRadiusFraction = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Spawner Properties")
	float ActivateRadiusFraction = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Spawner Properties")
	float DeactivateRadiusFraction = 0.35f;

	UPROPERTY(VisibleInstanceOnly, Category = "Spawner Properties")
	TObjectPtr<ASlashCharacter> OverlappingCharacter;

	UPROPERTY(EditAnywhere, Category = "Spawner Properties")
	float InitialSphereOverlapDelay = 0.1f;

	UPROPERTY()
	bool bEnemiesVisible = false;

	UPROPERTY()
	bool bEnemiesSpawned = false;

	UPROPERTY()
	bool bEnemiesActive = false;
};
