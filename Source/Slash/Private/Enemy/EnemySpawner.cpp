// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/EnemySpawner.h"
#include "Characters/SlashCharacter.h"
#include "Components/SphereComponent.h"
#include "Enemy/Enemy.h"
#include "Enemy/EnemySpawnPoint.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());

	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleOverlappingCharacter();
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemySpawner::OnSphereBeginOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AEnemySpawner::OnSphereEndOverlap);

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemySpawner::OnInitialSphereOverlap,
	                                InitialSphereOverlapDelay, false);
}

void AEnemySpawner::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter && OverlappingCharacter == nullptr)
	{
		OverlappingCharacter = SlashCharacter;
		SetActorTickEnabled(true);
	}
}

void AEnemySpawner::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter && OverlappingCharacter)
	{
		OverlappingCharacter = nullptr;
		SetActorTickEnabled(false);
	}
}

void AEnemySpawner::OnInitialSphereOverlap()
{
	TArray<AActor*> OverlappingActors;
	Sphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(Actor);
		if (SlashCharacter && OverlappingCharacter == nullptr)
		{
			OverlappingCharacter = SlashCharacter;
			SetActorTickEnabled(true);
		}
	}
}

void AEnemySpawner::HandleOverlappingCharacter()
{
	if (Sphere && OverlappingCharacter)
	{
		const float DistanceToCharacter = FVector::Dist(GetActorLocation(), OverlappingCharacter->GetActorLocation());
		const float SphereRadius = Sphere->GetScaledSphereRadius();

		HandleShowingEnemies(DistanceToCharacter, SphereRadius);
		HandleSpawningEnemies(DistanceToCharacter, SphereRadius);
		HandleActivatingEnemies(DistanceToCharacter, SphereRadius);
	}
}

void AEnemySpawner::HandleShowingEnemies(const float DistanceToCharacter, const float SphereRadius)
{
	const float ShowRadius = SphereRadius * ShowRadiusFraction;
	const float HideRadius = SphereRadius * HideRadiusFraction;

	if (DistanceToCharacter <= ShowRadius && !bEnemiesVisible)
	{
		ShowEnemies();
		bEnemiesVisible = true;
	}
	else if (DistanceToCharacter > HideRadius && bEnemiesVisible)
	{
		HideEnemies();
		bEnemiesVisible = false;
	}
}

void AEnemySpawner::HandleSpawningEnemies(const float DistanceToCharacter, const float SphereRadius)
{
	const float SpawnRadius = SphereRadius * SpawnRadiusFraction;
	const float DespawnRadius = SphereRadius * DespawnRadiusFraction;

	if (bEnemiesVisible)
	{
		if (DistanceToCharacter <= SpawnRadius && !bEnemiesSpawned)
		{
			SpawnEnemies();
			bEnemiesSpawned = true;
		}
		else if (DistanceToCharacter > DespawnRadius && bEnemiesSpawned)
		{
			DespawnEnemies();
			bEnemiesSpawned = false;
		}
	}
}

void AEnemySpawner::HandleActivatingEnemies(const float DistanceToCharacter, const float SphereRadius)
{
	const float ActivateRadius = SphereRadius * ActivateRadiusFraction;
	const float DeactivateRadius = SphereRadius * DeactivateRadiusFraction;

	if (bEnemiesSpawned)
	{
		if (DistanceToCharacter <= ActivateRadius && !bEnemiesActive)
		{
			ActivateEnemies();
			bEnemiesActive = true;
		}
		else if (DistanceToCharacter > DeactivateRadius && bEnemiesActive)
		{
			DeactivateEnemies();
			bEnemiesActive = false;
		}
	}
}

void AEnemySpawner::ShowEnemies()
{
	for (const auto SpawnPoint : SpawnPoints)
	{
		SpawnPoint->ShowEnemy();
	}
}

void AEnemySpawner::HideEnemies()
{
	for (const auto SpawnPoint : SpawnPoints)
	{
		SpawnPoint->HideEnemy();
	}
}

void AEnemySpawner::SpawnEnemies()
{
	for (const auto SpawnPoint : SpawnPoints)
	{
		if (const TObjectPtr<AEnemy> SpawnedEnemy = SpawnPoint->SpawnEnemy())
		{
			SpawnedEnemies.Add(SpawnedEnemy);
		}
	}

	UpdateAlliedEnemies();
}

void AEnemySpawner::DespawnEnemies()
{
	for (auto SpawnPoint : SpawnPoints)
	{
		SpawnPoint->DespawnEnemy();
	}

	SpawnedEnemies.Empty();
}

void AEnemySpawner::UpdateAlliedEnemies()
{
	for (const auto SpawnedEnemy : SpawnedEnemies)
	{
		TArray<TObjectPtr<AEnemy>> AlliedEnemies;
		for (const auto Enemy : SpawnedEnemies)
		{
			if (Enemy != SpawnedEnemy)
			{
				AlliedEnemies.Add(Enemy);
			}
		}

		SpawnedEnemy->SetAlliedEnemies(AlliedEnemies);
	}
}

void AEnemySpawner::ActivateEnemies()
{
	for (auto SpawnPoint : SpawnPoints)
	{
		SpawnPoint->ActivateEnemy();
	}
}

void AEnemySpawner::DeactivateEnemies()
{
	for (auto SpawnPoint : SpawnPoints)
	{
		SpawnPoint->DeactivateEnemy();
	}
}
