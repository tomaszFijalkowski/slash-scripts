// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void RegenHealth(const float DeltaTime);
	void RegenStamina(const float DeltaTime);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float HealthRegenRate = 1.f;

	// Modified by poison
	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	float PoisonDamagePerSecond = 0.f;

	// Modified by potions
	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	float HealthRegenRateMultiplier = 1.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float CurrentStamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StaminaRegenRate = 5.f;

	// Modified by potions
	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	float StaminaRegenRateMultiplier = 1.f;

	// Modified by potions or chill
	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	float WalkSpeedMultiplier = 1.f;

	// Modified by potions or chill
	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	float AnimationRateScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float AttackStaminaCost = 12.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float DodgeStaminaCost = 20.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Souls = 0;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Gold = 0;

public:
	float GetHealthPercent();
	float GetStaminaPercent();

	bool IsAlive();

	void Heal(float HealthAmount);
	void ReceiveDamage(float Damage);
	void ReceivePoisonDamageOverTime(const float DeltaTime);

	void RecoverStamina(float StaminaAmount);
	void ConsumeStamina(float StaminaCost);

	void AddSouls(int32 Amount);
	void AddGold(int32 Amount);

	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
	FORCEINLINE void SetCurrentHealth(const float Health) { CurrentHealth = Health; }
	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina; }
	FORCEINLINE float GetAttackStaminaCost() const { return AttackStaminaCost; }
	FORCEINLINE float GetDodgeStaminaCost() const { return DodgeStaminaCost; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE float GetHealthRegenRateMultiplier() const { return HealthRegenRateMultiplier; }
	FORCEINLINE void SetHealthRegenRateMultiplier(const float Multiplier) { HealthRegenRateMultiplier = Multiplier; }
	FORCEINLINE float GetPoisonDamagePerSecond() const { return PoisonDamagePerSecond; }
	FORCEINLINE void SetPoisonDamagePerSecond(const float Damage) { PoisonDamagePerSecond = Damage; }
	FORCEINLINE float GetStaminaRegenRateMultiplier() const { return StaminaRegenRateMultiplier; }
	FORCEINLINE void SetStaminaRegenRateMultiplier(const float Multiplier) { StaminaRegenRateMultiplier = Multiplier; }
	FORCEINLINE float GetWalkSpeedMultiplier() const { return WalkSpeedMultiplier; }
	FORCEINLINE void SetWalkSpeedMultiplier(const float Multiplier) { WalkSpeedMultiplier = Multiplier; }
	FORCEINLINE float GetAnimationRateScale() const { return AnimationRateScale; }
	FORCEINLINE void SetAnimationRateScale(const float RateScale) { AnimationRateScale = RateScale; }
};
