// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

struct FInputActionValue;
class UNiagaraComponent;
class UAttributeComponent;
class AWeapon;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void VampiricHeal(const float HealthAmount);
	virtual void VampiricHit();
	virtual void ApplyChill(const float SlowMultiplier, const float Duration);
	virtual void ApplyPoison(const float Damage, const float Duration);
	virtual bool IsAlive();

protected:
	// Combat
	virtual void BeginPlay() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, ABaseCharacter* Hitter) override;
	virtual void HandleDamage(const float DamageAmount);
	virtual void HandlePoisonDamageOverTime(const float DeltaTime);

	void DisableCapsule();

	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);

	void UpdateMaxWalkSpeed();

	// Montage
	void PlayHitReactMontage(FName SectionName);
	void PlayAttackMontage(FName SectionName = NAME_None);
	int32 PlayDeathMontage();
	void StopAttackMontage();

	void PlayMontageSection(UAnimMontage* const Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* const Montage, const TArray<FName>& SectionNames);

	UFUNCTION(BlueprintNativeEvent)
	void Die();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	virtual void HitReactEnd();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(const ECollisionEnabled::Type CollisionEnabled);

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraComponent> VampiricHealEffect;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraComponent> VampiricHitEffect;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraComponent> ChillEffect;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraComponent> PoisonEffect;

	UPROPERTY()
	bool bIsChilled = false;

	UPROPERTY()
	bool bIsPoisoned = false;

	UPROPERTY()
	float CurrentSpeed;

private:
	void StartAmbientSoundTimer();
	void PlayAmbientSound();
	void StopAmbientSound();
	void UpdateAttackTrailEffect();
	void DisableMeshCollision();
	void HandleWeaponOnHitEffects(ABaseCharacter* Hitter);
	void ShowPoisonEffect();
	void ShowChillEffect();
	void ApplyChillEnd();
	void ApplyPoisonEnd();
	void StopModifierEffects();

	UPROPERTY()
	FTimerHandle AmbientSoundTimer;

	UPROPERTY()
	FTimerHandle ChillTimer;

	UPROPERTY()
	FTimerHandle PoisonTimer;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> AmbientSound;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	TObjectPtr<UAudioComponent> AudioComponent;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UParticleSystem> HitParticles;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> AttackMontageSections;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageSections;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
