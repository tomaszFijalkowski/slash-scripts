// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnPoint.generated.h"

class ASkeletalMeshActor;
class ATargetPoint;
class AEnemy;

UCLASS()
class SLASH_API AEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawnPoint();
	virtual void Tick(float DeltaTime) override;
	void ShowEnemy();
	void HideEnemy();

	TObjectPtr<AEnemy> SpawnEnemy();
	void DespawnEnemy();

	void ActivateEnemy();
	void DeactivateEnemy();

	void OnEnemyDeath();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnSkeletalMesh();

	UPROPERTY(EditAnywhere, Category = "Enemy Properties")
	float MaxAllowedDistance = 8000;

	UPROPERTY(EditAnywhere, Category = "Enemy Properties")
	TObjectPtr<UAnimSequence> IdlePoseClass;

	UPROPERTY(EditAnywhere, Category = "Enemy Properties")
	TObjectPtr<USkeletalMesh> SkeletalMeshClass;

	UPROPERTY(EditAnywhere, Category = "Enemy Properties")
	FVector SkeletalMeshLocationOffset = FVector(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "Enemy Properties")
	FRotator SkeletalMeshRotationOffset = FRotator(0.f, -90.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "Enemy Properties")
	TSubclassOf<AEnemy> EnemyClass;

	UPROPERTY(EditInstanceOnly, Category = "Enemy Properties")
	TArray<TObjectPtr<ATargetPoint>> EnemyPatrolTargets;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	TObjectPtr<ASkeletalMeshActor> SpawnedSkeletalMesh;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	TObjectPtr<AEnemy> SpawnedEnemy;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	FVector SavedEnemyLocation;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	FRotator SavedEnemyRotation;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	TObjectPtr<ATargetPoint> SavedEnemyCurrentPatrolTarget;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	float SavedEnemyCurrentHealth = 0.f;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	bool bIsInitialSpawn = true;

	UPROPERTY(VisibleInstanceOnly, Category = "Enemy Properties")
	bool bIsEnabled = true;

public:
	FORCEINLINE float GetMaxAllowedDistance() const { return MaxAllowedDistance; }
};
