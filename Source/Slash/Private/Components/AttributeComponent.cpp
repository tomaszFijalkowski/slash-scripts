// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

float UAttributeComponent::GetHealthPercent()
{
	return CurrentHealth / MaxHealth;
}

float UAttributeComponent::GetStaminaPercent()
{
	return CurrentStamina / MaxStamina;
}

bool UAttributeComponent::IsAlive()
{
	return CurrentHealth > 0.f;
}

void UAttributeComponent::Heal(const float HealthAmount)
{
	CurrentHealth = FMath::Clamp<float>(CurrentHealth + HealthAmount, 0.f, MaxHealth);
}

void UAttributeComponent::ReceiveDamage(const float Damage)
{
	CurrentHealth = FMath::Clamp<float>(CurrentHealth - Damage, 0.f, MaxHealth);
}

void UAttributeComponent::ReceivePoisonDamageOverTime(const float DeltaTime)
{
	const float HealthAfterDamageTick = CurrentHealth - PoisonDamagePerSecond * DeltaTime;
	CurrentHealth = FMath::Clamp<float>(HealthAfterDamageTick, 0, MaxHealth);
}

void UAttributeComponent::RecoverStamina(const float StaminaAmount)
{
	CurrentStamina = FMath::Clamp<float>(CurrentStamina + StaminaAmount, 0.f, MaxStamina);
}

void UAttributeComponent::ConsumeStamina(const float StaminaCost)
{
	CurrentStamina = FMath::Clamp<float>(CurrentStamina - StaminaCost, 0.f, MaxStamina);
}

void UAttributeComponent::AddSouls(const int32 Amount)
{
	Souls += Amount;
}

void UAttributeComponent::AddGold(const int32 Amount)
{
	Gold += Amount;
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAttributeComponent::RegenHealth(const float DeltaTime)
{
	const float HealthAfterRegen = CurrentHealth + HealthRegenRate * HealthRegenRateMultiplier * DeltaTime;
	CurrentHealth = FMath::Clamp<float>(HealthAfterRegen, 0, MaxHealth);
}

void UAttributeComponent::RegenStamina(const float DeltaTime)
{
	const float StaminaAfterRegen = CurrentStamina + StaminaRegenRate * StaminaRegenRateMultiplier * DeltaTime;
	CurrentStamina = FMath::Clamp<float>(StaminaAfterRegen, 0, MaxStamina);
}
