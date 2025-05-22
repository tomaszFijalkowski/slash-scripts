// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BaseCharacter.h"
#include "NiagaraComponent.h"
#include "Animation/AnimNotifies/AnimNotifyState_Trail.h"
#include "Components/AttributeComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Constants/Animation.h"
#include "Constants/GameplayTag.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Weapons/Modifiers/WeaponModifier.h"
#include "Kismet/GameplayStatics.h"
#include "Slash/DebugMacros.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));

	VampiricHealEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VampiricHealEffect"));
	VampiricHealEffect->SetupAttachment(GetRootComponent());
	VampiricHealEffect->SetAutoActivate(false);

	VampiricHitEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VampiricHitEffect"));
	VampiricHitEffect->SetupAttachment(GetRootComponent());
	VampiricHitEffect->SetAutoActivate(false);

	ChillEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChillEffect"));
	ChillEffect->SetupAttachment(GetRootComponent());
	ChillEffect->SetAutoActivate(false);

	PoisonEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PoisonEffect"));
	PoisonEffect->SetupAttachment(GetRootComponent());
	PoisonEffect->SetAutoActivate(false);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandlePoisonDamageOverTime(DeltaTime);
}

void ABaseCharacter::VampiricHeal(const float HealthAmount)
{
	if (Attributes)
	{
		Attributes->Heal(HealthAmount);
	}

	if (VampiricHealEffect)
	{
		VampiricHealEffect->Activate(true);
	}
}

void ABaseCharacter::VampiricHit()
{
	if (VampiricHitEffect)
	{
		VampiricHitEffect->Activate(true);
	}
}

void ABaseCharacter::ApplyChill(const float SlowMultiplier, const float Duration)
{
	if (Attributes && Attributes->IsAlive() && Duration > 0)
	{
		bIsChilled = true;

		Attributes->SetWalkSpeedMultiplier(SlowMultiplier);
		Attributes->SetAnimationRateScale(SlowMultiplier);
		UpdateMaxWalkSpeed();
		ShowChillEffect();
		GetWorldTimerManager().SetTimer(
			ChillTimer,
			this,
			&ABaseCharacter::ApplyChillEnd,
			Duration
		);
	}
}

void ABaseCharacter::ApplyPoison(const float Damage, const float Duration)
{
	if (Attributes && Attributes->IsAlive() && Duration > 0)
	{
		bIsPoisoned = true;
		Attributes->SetPoisonDamagePerSecond(Damage / Duration);
		ShowPoisonEffect();
		GetWorldTimerManager().SetTimer(
			PoisonTimer,
			this,
			&ABaseCharacter::ApplyPoisonEnd,
			Duration
		);
	}
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	StartAmbientSoundTimer();
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, ABaseCharacter* Hitter)
{
	if (Hitter == nullptr)
	{
		return;
	}

	// To prevent the weapon from never disabling collision if interrupted during the attack animation
	// (after EnableBoxCollision and before DisableBoxCollision notifies)
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	HandleWeaponOnHitEffects(Hitter);

	if (IsAlive())
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else
	{
		Die();
	}

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::HandleDamage(const float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::HandlePoisonDamageOverTime(const float DeltaTime)
{
	if (Attributes && bIsPoisoned)
	{
		Attributes->ReceivePoisonDamageOverTime(DeltaTime);

		if (!Attributes->IsAlive())
		{
			Die();
		}
	}
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	// Lower Impact Point to the Enemy's Actor Location Z
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward||ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);

	// Take the inverse cosine (arc-cosine) of cos(theta) to get theta (angle between Forward and ToHit)
	const double ThetaInRadians = FMath::Acos(CosTheta);

	// Convert from radians to degrees
	double ThetaInDegrees = FMath::RadiansToDegrees(ThetaInRadians);

	// if CrossProduct points down, theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		ThetaInDegrees *= -1;
	}

	FName Section(Section::FromBack);

	if (ThetaInDegrees >= -45.f && ThetaInDegrees < 45.f)
	{
		Section = Section::FromFront;
	}
	else if (ThetaInDegrees >= -135.f && ThetaInDegrees < -45.f)
	{
		Section = Section::FromLeft;
	}
	else if (ThetaInDegrees >= 45.f && ThetaInDegrees < 135.f)
	{
		Section = Section::FromRight;
	}

	PlayHitReactMontage(Section);
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, ImpactPoint);
	}
}

void ABaseCharacter::UpdateMaxWalkSpeed()
{
	if (Attributes)
	{
		GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed * Attributes->GetWalkSpeedMultiplier();
	}
}

void ABaseCharacter::PlayHitReactMontage(const FName SectionName)
{
	if (HitReactMontage && Attributes)
	{
		HitReactMontage->RateScale = Attributes->GetAnimationRateScale();
		PlayMontageSection(HitReactMontage, SectionName);
	}
}

void ABaseCharacter::PlayAttackMontage(const FName SectionName)
{
	if (AttackMontage && Attributes)
	{
		AttackMontage->RateScale = Attributes->GetAnimationRateScale();
		UpdateAttackTrailEffect();
	}

	if (SectionName != NAME_None)
	{
		PlayMontageSection(AttackMontage, SectionName);
	}
	else
	{
		PlayRandomMontageSection(AttackMontage, AttackMontageSections);
	}
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	const TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}

	return Selection;
}

void ABaseCharacter::StopAttackMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* const Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* const Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0)
	{
		return -1;
	}
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);

	PlayMontageSection(Montage, SectionNames[Selection]);

	return Selection;
}

void ABaseCharacter::Die_Implementation()
{
	Tags.Add(GameplayTag::Dead);

	StopModifierEffects();

	DisableCapsule();
	DisableMeshCollision();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	PlayDeathMontage();
	StopAmbientSound();
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::HitReactEnd()
{
}

void ABaseCharacter::SetWeaponCollisionEnabled(const ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponBoxCollisionEnabled(CollisionEnabled);

		// Required for attacks with multiple swings
		if (CollisionEnabled == ECollisionEnabled::NoCollision)
		{
			EquippedWeapon->EmptyActorsToIgnore();
		}
	}
}

void ABaseCharacter::StartAmbientSoundTimer()
{
	if (AmbientSound)
	{
		const float StartDelay = FMath::RandRange(0.f, AmbientSound->Duration);
		GetWorldTimerManager().SetTimer(AmbientSoundTimer, this, &ABaseCharacter::PlayAmbientSound, StartDelay, false);
	}
}

void ABaseCharacter::PlayAmbientSound()
{
	if (AudioComponent && AmbientSound)
	{
		AudioComponent->SetSound(AmbientSound);
		AudioComponent->bAllowSpatialization = true;
		AudioComponent->bIsUISound = false;
		AudioComponent->Play();
	}
}

void ABaseCharacter::StopAmbientSound()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}

void ABaseCharacter::UpdateAttackTrailEffect()
{
	bool bCanUpdateAttackTrailEffect = AttackMontage && EquippedWeapon && EquippedWeapon->GetTrailEffect();
	if (bCanUpdateAttackTrailEffect)
	{
		for (auto Notify : AttackMontage->Notifies)
		{
			if (UAnimNotifyState_Trail* TrailNotify = Cast<UAnimNotifyState_Trail>(Notify.NotifyStateClass))
			{
				TrailNotify->PSTemplate = EquippedWeapon->GetTrailEffect();
			}
		}
	}
}

void ABaseCharacter::DisableMeshCollision()
{
	// Disable collision and overlap events to avoid taking hits while already playing DeathMontage
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetGenerateOverlapEvents(false);
}

void ABaseCharacter::HandleWeaponOnHitEffects(ABaseCharacter* Hitter)
{
	check(Hitter);

	if (const AWeapon* Weapon = Hitter->EquippedWeapon)
	{
		if (UWeaponModifier* WeaponModifier = Weapon->GetModifier())
		{
			WeaponModifier->OnHitEffect(Hitter, this);
		}
	}
}

void ABaseCharacter::ShowPoisonEffect()
{
	if (PoisonEffect)
	{
		PoisonEffect->Activate();
	}
}

void ABaseCharacter::ShowChillEffect()
{
	if (ChillEffect)
	{
		ChillEffect->Activate();
	}
}

void ABaseCharacter::ApplyChillEnd()
{
	if (Attributes)
	{
		Attributes->SetWalkSpeedMultiplier(1.f);
		Attributes->SetAnimationRateScale(1.f);
		UpdateMaxWalkSpeed();
	}

	if (HitReactMontage)
	{
		HitReactMontage->RateScale = 1.f;
	}

	if (AttackMontage)
	{
		AttackMontage->RateScale = 1.f;
	}

	if (ChillEffect)
	{
		ChillEffect->Deactivate();
	}
}

void ABaseCharacter::ApplyPoisonEnd()
{
	bIsPoisoned = false;

	if (Attributes)
	{
		Attributes->SetPoisonDamagePerSecond(0.f);
	}

	if (PoisonEffect)
	{
		PoisonEffect->Deactivate();
	}
}

void ABaseCharacter::StopModifierEffects()
{
	if (bIsChilled)
	{
		ApplyChillEnd();
	}

	if (bIsPoisoned)
	{
		ApplyPoisonEnd();
	}
}
