// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Enemy.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Characters/CharacterTargetPoint.h"
#include "Characters/SlashCharacter.h"
#include "Components/AttributeComponent.h"
#include "Components/SphereComponent.h"
#include "Constants/GameplayTag.h"
#include "Constants/Socket.h"
#include "Enemy/EnemySpawnPoint.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Soul.h"
#include "Items/Weapons/Weapon.h"
#include "Perception/PawnSensingComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent());

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 1000.f;
	PawnSensing->SetPeripheralVisionAngle(75.f);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
	{
		return;
	}

	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
}

float AEnemy::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
                         AActor* DamageCauser)
{
	ClearPatrolTimer();
	ClearAttackTimer();
	HandleDamage(DamageAmount);
	SetCombatTarget(EventInstigator->GetPawn());
	AlertNearbyAllies();

	return DamageAmount;
}

void AEnemy::Destroyed()
{
	Super::Destroyed();

	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, ABaseCharacter* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
}

void AEnemy::Alert(APawn* SeenPawn)
{
	const bool bIsAlreadyAlerted = GetWorldTimerManager().IsTimerActive(AlertTimer);
	if (ShouldRespondToAlert(SeenPawn) && !bIsAlreadyAlerted)
	{
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([this, SeenPawn]
		{
			// Check if still valid after the delay
			if (ShouldRespondToAlert(SeenPawn))
			{
				PawnSeen(SeenPawn);
			}
		});

		GetWorldTimerManager().SetTimer(AlertTimer, TimerDelegate, AlertDelay, false);
	}
}

void AEnemy::StartPatrolling()
{
	if (Attributes)
	{
		EnemyState = EEnemyState::EES_Patrolling;
		CurrentSpeed = PatrolSpeed;
		UpdateMaxWalkSpeed();
		MoveToTarget(CurrentPatrolTarget, PatrolAcceptanceRadius);
	}
}

void AEnemy::Activate()
{
	SetActorTickEnabled(true);
	PawnSensing->SetSensingUpdatesEnabled(true);
}

void AEnemy::Deactivate()
{
	SetActorTickEnabled(false);
	PawnSensing->SetSensingUpdatesEnabled(false);
}

float AEnemy::GetCurrentHealth()
{
	return Attributes ? Attributes->GetCurrentHealth() : 0.f;
}

void AEnemy::SetCurrentHealth(const float Health)
{
	if (Attributes)
	{
		Attributes->SetCurrentHealth(Health);
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(GameplayTag::Enemy);

	EnemyController = Cast<AAIController>(GetController());
	if (EnemyController)
	{
		EnemyController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &AEnemy::OnMoveCompleted);
	}

	if (HealthBarComponent && Attributes)
	{
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
		HideHealthBar();
	}

	if (PawnSensing)
	{
		PawnSensing->SetSensingUpdatesEnabled(false);
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	if (Sphere)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnSphereBeginOverlap);
		Sphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnSphereEndOverlap);
	}

	SpawnDefaultWeapon();
	StartClosestTargetPointUpdateTimer();
}

void AEnemy::Die_Implementation()
{
	Super::Die_Implementation();

	EnemyState = EEnemyState::EES_Dead;
	Sphere->SetGenerateOverlapEvents(false);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	ClearCombatTarget();
	ClearAttackTimer();
	HideHealthBar();
	SpawnSoul();

	SpawnPoint->OnEnemyDeath();
	SetLifeSpan(DeathLifeSpan);
}

void AEnemy::AttackEnd()
{
	Super::AttackEnd();

	EnemyState = EEnemyState::EES_NoState;

	if (EquippedWeapon)
	{
		EquippedWeapon->EmptyActorsToIgnore();
	}

	UpdateClosestTargetPointToOccupy();
	CheckCombatTarget();
}

void AEnemy::HitReactEnd()
{
	Super::HitReactEnd();

	if (!IsDead())
	{
		ChaseTarget();
	}
}

void AEnemy::HandleDamage(const float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (HealthBarComponent && Attributes)
	{
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
		ShowHealthBar();
	}
}

void AEnemy::HandlePoisonDamageOverTime(const float DeltaTime)
{
	Super::HandlePoisonDamageOverTime(DeltaTime);

	if (HealthBarComponent && Attributes && bIsPoisoned)
	{
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
		ShowHealthBar();
	}
}

void AEnemy::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor))
	{
		SlashCharacter->AddOverlappingEnemy(this);
	}
}

void AEnemy::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor))
	{
		SlashCharacter->RemoveOverlappingEnemy(this);
	}
}

void AEnemy::SelectWarpTargetEnd()
{
	bShouldSelectWarpTarget = false;
	bHasExceededWarpDistance = false;
}

FVector AEnemy::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr || OccupiedTargetPoint == nullptr)
	{
		return FVector();
	}

	const FVector Location = GetActorLocation();
	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector OccupiedTargetPointLocation = OccupiedTargetPoint->GetActorLocation();

	const float DistanceToTarget = FVector::Dist(Location, OccupiedTargetPointLocation);
	const bool bShouldWarpTowardTarget =
		DistanceToTarget <= MaxWarpDistance &&
		!bHasExceededWarpDistance;

	if (bShouldWarpTowardTarget)
	{
		const FVector TargetToMe = (OccupiedTargetPointLocation - CombatTargetLocation).GetSafeNormal();
		const FVector WarpTargetOffsetToAdd = TargetToMe * WarpTargetOffset;
		WarpTarget = OccupiedTargetPointLocation + WarpTargetOffsetToAdd;
		// DRAW_SPHERE_SINGLE_FRAME_COLOR(WarpTarget, FColor::Green);
	}
	else
	{
		bHasExceededWarpDistance = true;

		if (bShouldSelectWarpTarget)
		{
			const FVector TargetFromMe = (CombatTargetLocation - Location).GetSafeNormal();
			WarpTarget = Location + TargetFromMe * MaxWarpDistance;
			// DRAW_SPHERE_SINGLE_FRAME_COLOR(WarpTarget, FColor::Yellow);
		}
		// else { DRAW_SPHERE_SINGLE_FRAME_COLOR(WarpTarget, FColor::Red); }
	}

	return WarpTarget;
}

FVector AEnemy::GetRotationWarpTarget()
{
	return CombatTarget ? CombatTarget->GetActorLocation() : FVector();
}

void AEnemy::HideHealthBar()
{
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(true);
	}
}

void AEnemy::CheckCombatTarget()
{
	if (IsOutsideCombatRadius() && !IsPatrolling())
	{
		ClearAttackTimer();
		LoseInterest();

		if (!IsEngaged())
		{
			StartPatrolling();
		}
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();

		if (!IsEngaged())
		{
			ChaseTarget();
			AlertNearbyAllies();
		}
	}
	else if (CanAttack())
	{
		StartAttackTimer();
	}
}

void AEnemy::CheckPatrolTarget()
{
	if (CurrentPatrolTarget == nullptr || IsInPatrolAcceptanceRadius())
	{
		CurrentPatrolTarget = ChoosePatrolTarget();
		const float PatrolWaitTime = FMath::RandRange(PatrolWaitTimeMin, PatrolWaitTimeMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, PatrolWaitTime);
	}
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsPatrolling()
{
	return EnemyState == EEnemyState::EES_Patrolling;
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

bool AEnemy::IsInPatrolAcceptanceRadius()
{
	const float AcceptanceRadiusMargin = PatrolAcceptanceRadius + PatrolAcceptanceRadiusMargin;
	return InTargetRange(CurrentPatrolTarget, AcceptanceRadiusMargin, true);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::ShouldRespondToAlert(const APawn* SeenPawn)
{
	return SeenPawn && InTargetRange(SeenPawn, CombatRadius) && CombatTarget == nullptr;
}

void AEnemy::LoseInterest()
{
	ClearCombatTarget();
	HideHealthBar();
}

void AEnemy::ChaseTarget()
{
	if (Attributes)
	{
		EnemyState = EEnemyState::EES_Chasing;
		CurrentSpeed = ChaseSpeed;
		UpdateMaxWalkSpeed();
		SetClosestTargetPointToOccupy();
		MoveToTarget(OccupiedTargetPoint, ChaseAcceptanceRadius);
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(CurrentPatrolTarget, PatrolAcceptanceRadius);
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::SetCombatTarget(APawn* Pawn)
{
	CombatTarget = Cast<ASlashCharacter>(Pawn);
}

void AEnemy::ClearCombatTarget()
{
	if (OccupiedTargetPoint)
	{
		OccupiedTargetPoint->MarkBothAsUnoccupied();
		OccupiedTargetPoint = nullptr;
	}

	CombatTarget = nullptr;
}

void AEnemy::StartClosestTargetPointUpdateTimer()
{
	GetWorldTimerManager().SetTimer(ClosestTargetPointUpdateTimer, this, &AEnemy::UpdateClosestTargetPointToOccupy,
	                                ClosestTargetPointUpdateTimerFrequency, true);
}

bool AEnemy::CanUpdateClosestTargetPoint()
{
	return CombatTarget && MoveRequest.IsValid() && !IsEngaged();
}

void AEnemy::SetClosestTargetPointToOccupy()
{
	if (OccupiedTargetPoint)
	{
		OccupiedTargetPoint->MarkBothAsUnoccupied();
		OccupiedTargetPoint = nullptr;
	}

	OccupiedTargetPoint = FindClosestTargetPointToOccupy();
}

void AEnemy::UpdateClosestTargetPointToOccupy()
{
	const bool bIsInMaxAllowedRange = InTargetRange(SpawnPoint, SpawnPoint->GetMaxAllowedDistance(), true);
	if (CanUpdateClosestTargetPoint() && bIsInMaxAllowedRange)
	{
		ACharacterTargetPoint* NewClosestTargetPoint = FindClosestTargetPointToOccupy();
		if (NewClosestTargetPoint != OccupiedTargetPoint)
		{
			if (OccupiedTargetPoint)
			{
				OccupiedTargetPoint->MarkBothAsUnoccupied();
			}
			OccupiedTargetPoint = NewClosestTargetPoint;
			MoveRequest.SetGoalActor(OccupiedTargetPoint);
		}
	}
	else if (!bIsInMaxAllowedRange)
	{
		LoseInterest();
	}
}

ACharacterTargetPoint* AEnemy::FindClosestTargetPointToOccupy()
{
	float ShortestDistance = FLT_MAX;
	ACharacterTargetPoint* ClosestTargetPoint = nullptr;
	TArray<TObjectPtr<ACharacterTargetPoint>> TargetPoints = ShouldOccupyInnerTargetPoint
		                                                         ? CombatTarget->GetInnerTargetPoints()
		                                                         : CombatTarget->GetOuterTargetPoints();

	for (const auto TargetPoint : TargetPoints)
	{
		const float Distance = FVector::Dist(GetActorLocation(), TargetPoint->GetActorLocation());
		const bool IsValidTargetPoint = TargetPoint == OccupiedTargetPoint || !TargetPoint->IsOccupied();
		if (IsValidTargetPoint && Distance < ShortestDistance)
		{
			ShortestDistance = Distance;
			ClosestTargetPoint = TargetPoint;
		}
	}

	if (ClosestTargetPoint && ClosestTargetPoint != OccupiedTargetPoint)
	{
		ClosestTargetPoint->MarkBothAsOccupied();
	}

	return ClosestTargetPoint;
}

bool AEnemy::CanAttack()
{
	const bool bCanAttack =
		IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsEngaged() &&
		!IsDead();

	return bCanAttack;
}

void AEnemy::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(GameplayTag::Dead))
	{
		ClearCombatTarget();
	}

	if (CombatTarget == nullptr)
	{
		return;
	}

	EnemyState = EEnemyState::EES_Engaged;
	bShouldSelectWarpTarget = true;
	bHasExceededWarpDistance = false;

	PlayAttackMontage();
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

ATargetPoint* AEnemy::ChoosePatrolTarget()
{
	TArray<ATargetPoint*> ValidTargets;
	for (auto Target : PatrolTargets)
	{
		if (Target != CurrentPatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}

	if (const int32 NumValidTargets = ValidTargets.Num())
	{
		const int32 TargetSelection = FMath::RandRange(0, NumValidTargets - 1);
		return ValidTargets[TargetSelection];
	}

	return nullptr;
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();

	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), Socket::Weapon, this, this, false);
		EquippedWeapon = DefaultWeapon;
	}
}

bool AEnemy::InTargetRange(const AActor* Target, const double Radius, const bool bIgnoreElevation)
{
	if (Target == nullptr)
	{
		return false;
	}

	FVector TargetLocation = Target->GetActorLocation();
	const FVector EnemyLocation = GetActorLocation();

	if (bIgnoreElevation)
	{
		TargetLocation.Z = EnemyLocation.Z;
	}

	const double DistanceToTarget = (TargetLocation - EnemyLocation).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(const ATargetPoint* Target, const float AcceptanceRadius)
{
	if (EnemyController == nullptr || Target == nullptr)
	{
		return;
	}

	MoveRequest = FAIMoveRequest();
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	MoveRequest.SetAllowPartialPath(true);

	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass && Attributes)
	{
		FTransform SpawnTransform = GetActorTransform();
		const FVector LocationOffset = FVector(0.f, 0.f, 300.f);
		SpawnTransform.SetLocation(SpawnTransform.GetLocation() + LocationOffset);

		if (ASoul* Soul = World->SpawnActorDeferred<ASoul>(SoulClass, SpawnTransform, this))
		{
			Soul->SetSouls(Attributes->GetSouls());
			Soul->FinishSpawning(SpawnTransform);
		}
	}
}

void AEnemy::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (IsDead())
	{
		return;
	}

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSystem)
	{
		FNavLocation NavLocation;
		const bool bOnNavMesh = OccupiedTargetPoint && NavSystem->ProjectPointToNavigation(
			OccupiedTargetPoint->GetActorLocation(), NavLocation,
			FVector(0.f, 0.f, 500.f));

		if (!bOnNavMesh)
		{
			LoseInterest();
		}
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		SeenPawn->ActorHasTag(GameplayTag::EngageableTarget) &&
		SeenPawn->ActorHasTag(GameplayTag::Dead) == false;

	if (bShouldChaseTarget)
	{
		SetCombatTarget(SeenPawn);
		ClearPatrolTimer();
		ChaseTarget();
	}

	AlertNearbyAllies();
}

void AEnemy::AlertNearbyAllies()
{
	for (const auto Enemy : AlliedEnemies)
	{
		if (Enemy && Enemy != this && InTargetRange(Enemy, AlertRadius))
		{
			Enemy->Alert(CombatTarget);
		}
	}
}
