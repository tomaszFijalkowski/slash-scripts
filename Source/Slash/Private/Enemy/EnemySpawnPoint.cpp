// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/EnemySpawnPoint.h"
#include "Animation/SkeletalMeshActor.h"
#include "Enemy/Enemy.h"

AEnemySpawnPoint::AEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	SavedEnemyLocation = GetActorLocation();
	SavedEnemyRotation = GetActorRotation();
}

void AEnemySpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemySpawnPoint::ShowEnemy()
{
	if (bIsEnabled && SkeletalMeshClass)
	{
		SpawnSkeletalMesh();
	}
}

void AEnemySpawnPoint::HideEnemy()
{
	if (bIsEnabled && SkeletalMeshClass)
	{
		SpawnedSkeletalMesh->Destroy();
	}
}

TObjectPtr<AEnemy> AEnemySpawnPoint::SpawnEnemy()
{
	UWorld* World = GetWorld();
	if (bIsEnabled && World && EnemyClass)
	{
		if (SpawnedSkeletalMesh)
		{
			SpawnedSkeletalMesh->Destroy();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnedEnemy = World->SpawnActor<AEnemy>(EnemyClass, SavedEnemyLocation, SavedEnemyRotation, SpawnParams);
		if (SpawnedEnemy)
		{
			SpawnedEnemy->SetSpawnPoint(this);
			SpawnedEnemy->SetPatrolTargets(EnemyPatrolTargets);

			if (!bIsInitialSpawn)
			{
				SpawnedEnemy->SetCurrentHealth(SavedEnemyCurrentHealth);
				SpawnedEnemy->SetCurrentPatrolTarget(SavedEnemyCurrentPatrolTarget);
				SpawnedEnemy->StartPatrolling();
			}

			return SpawnedEnemy;
		}
	}

	return nullptr;
}

void AEnemySpawnPoint::DespawnEnemy()
{
	if (bIsEnabled && SpawnedEnemy && SpawnedEnemy->IsAlive())
	{
		SavedEnemyLocation = SpawnedEnemy->GetActorLocation();
		SavedEnemyRotation = SpawnedEnemy->GetActorRotation();
		SavedEnemyCurrentHealth = SpawnedEnemy->GetCurrentHealth();
		SavedEnemyCurrentPatrolTarget = SpawnedEnemy->GetCurrentPatrolTarget();

		SpawnedEnemy->Destroy();
		SpawnSkeletalMesh();

		bIsInitialSpawn = false;
	}
}

void AEnemySpawnPoint::ActivateEnemy()
{
	if (bIsEnabled && SpawnedEnemy && SpawnedEnemy->IsAlive())
	{
		SpawnedEnemy->Activate();
	}
}

void AEnemySpawnPoint::DeactivateEnemy()
{
	if (bIsEnabled && SpawnedEnemy && SpawnedEnemy->IsAlive())
	{
		SpawnedEnemy->Deactivate();
	}
}

void AEnemySpawnPoint::OnEnemyDeath()
{
	bIsEnabled = false;
}

void AEnemySpawnPoint::SpawnSkeletalMesh()
{
	UWorld* World = GetWorld();
	if (World && SkeletalMeshClass)
	{
		const FVector AdjustedLocation = SavedEnemyLocation + SkeletalMeshLocationOffset;
		const FRotator AdjustedRotation = SavedEnemyRotation + SkeletalMeshRotationOffset;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnedSkeletalMesh = World->SpawnActor<ASkeletalMeshActor>(ASkeletalMeshActor::StaticClass(),
		                                                            AdjustedLocation, AdjustedRotation, SpawnParams);
		if (SpawnedSkeletalMesh)
		{
			if (USkeletalMeshComponent* SkeletalMeshComp = SpawnedSkeletalMesh->GetSkeletalMeshComponent())
			{
				SkeletalMeshComp->SetSkeletalMesh(SkeletalMeshClass);

				if (IdlePoseClass)
				{
					SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
					SkeletalMeshComp->SetAnimation(IdlePoseClass);
				}
			}
		}
	}
}
