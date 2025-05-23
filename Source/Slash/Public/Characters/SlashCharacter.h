// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class AEnemy;
class ACharacterTargetPoint;
class USphereComponent;
class AFoliageManager;
class ABreakableActor;
class ATreasure;
class USlashOverlay;
class AItem;
class UGroomComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void VampiricHeal(const float HealthAmount) override;
	virtual float TakeDamage(const float DamageAmount, const FDamageEvent& DamageEvent,
	                         AController* EventInstigator, AActor* DamageCauser) override;
	// GetHit_Implementation is generated by UFUNCTION(BlueprintNativeEvent)
	virtual void GetHit_Implementation(const FVector& ImpactPoint, ABaseCharacter* Hitter) override;
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void PickupSoul(ASoul* Soul) override;
	virtual void PickupTreasure(ATreasure* Treasure) override;
	virtual bool CanUseHealthPotion() override;
	virtual void PickupHealthPotion(AHealthPotion* HealthPotion) override;
	virtual bool CanUseStaminaPotion() override;
	virtual void PickupStaminaPotion(AStaminaPotion* StaminaPotion) override;

	void AddOverlappingLowBreakable(ABreakableActor* Breakable);
	void RemoveOverlappingLowBreakable(ABreakableActor* Breakable);

	void AddOverlappingEnemy(AEnemy* Enemy);
	void RemoveOverlappingEnemy(AEnemy* Enemy);

protected:
	virtual void BeginPlay() override;

	// Enhanced input methods
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CustomJump(const FInputActionValue& Value);
	void Equip(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);
	void Dodge(const FInputActionValue& Value);
	void Scroll(const FInputActionValue& Value);
	void ExitGame(const FInputActionValue& Value);

	void EquipWeapon(AWeapon* OverlappingWeapon);
	void Disarm();
	void Arm();

	virtual void Die_Implementation() override;
	virtual void AttackEnd() override;
	virtual void HitReactEnd() override;

	bool CanAttack();
	bool CanDodge();
	bool CanDisarm();
	bool CanArm();

	void PlayDodgeMontage();
	void PlayEquipMontage(FName SectionName);

	UFUNCTION(BlueprintCallable)
	void DodgeEnd();

	UFUNCTION(BlueprintCallable)
	void EquipEnd();

	UFUNCTION(BlueprintCallable)
	void DisarmEnd();

	UFUNCTION(BlueprintCallable)
	void ArmEnd();

	UFUNCTION()
	void OnFoliageSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                 const FHitResult& SweepResult);

	UFUNCTION()
	void OnFoliageSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> SlashMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ScrollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ExitGameAction;

private:
	bool IsUnoccupied();
	bool IsDodging();
	bool IsCloseToLowBreakable();
	bool IsCloseToEnemy();

	void InitializeSlashOverlay(const APlayerController* PlayerController);
	void InitializeEnhancedInput(const APlayerController* PlayerController);

	void SetFoliageManager();

	void SpawnAndLinkTargetPoints();
	void SpawnTargetPoints(TArray<TObjectPtr<ACharacterTargetPoint>>& TargetPoints, const float TargetPointDistance);
	void LinkTargetPoints();
	void UpdateAllTargetPointPositions();
	void UpdateTargetPointPositions(TArray<TObjectPtr<ACharacterTargetPoint>>& TargetPoints,
	                                const float TargetPointDistance);
	FVector CalculateTargetPointLocation(const int32 Index, const float TargetPointDistance);

	void SetTargetZoomDistance(float DeltaTime);

	void SetHUDHealthBar();
	void SetHUDHealthBoostIcon();

	void RegenerateHealth(float DeltaTime);
	void BoostHealthRegenRate(const float Multiplier, const float Duration);
	void BoostHealthRegenRateEnd();

	void SetHUDStaminaBar();
	void SetHUDStaminaBoostIcon();

	void RegenerateStamina(float DeltaTime);
	void BoostStamina(const float RegenRateMultiplier, const float WalkSpeedMultiplier,
	                  const float AnimationRateScale,
	                  const float Duration);
	void BoostStaminaEnd();

	void ShowDeathScreen();
	void PlayDeathScreenSound();
	void RestartLevel();

	UPROPERTY()
	bool bShouldRegenerateHealth = false;

	UPROPERTY()
	FTimerHandle HealthBoostRegenRateTimer;

	UPROPERTY()
	FTimerHandle StaminaBoostRegenRateTimer;

	UPROPERTY()
	FTimerHandle DeathScreenSoundTimer;

	UPROPERTY()
	FTimerHandle RestartLevelTimer;

	UPROPERTY()
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float UnequippedWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float EquippedWalkSpeed = 575.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomSpeed = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float MinZoomDistance = 225.f;

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float MaxZoomDistance = 325.f;

	UPROPERTY()
	float TargetZoomDistance;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = "Groom")
	TObjectPtr<UGroomComponent> HairGroom;

	UPROPERTY(VisibleAnywhere, Category = "Groom")
	TObjectPtr<UGroomComponent> EyebrowsGroom;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AItem> OverlappingItem;

	UPROPERTY(VisibleInstanceOnly)
	TArray<TObjectPtr<ABreakableActor>> OverlappingLowBreakables;

	UPROPERTY(VisibleInstanceOnly)
	TArray<TObjectPtr<AEnemy>> OverlappingEnemies;

	UPROPERTY(VisibleAnywhere, Category = "AI Navigation")
	TArray<TObjectPtr<ACharacterTargetPoint>> InnerTargetPoints;

	UPROPERTY(VisibleAnywhere, Category = "AI Navigation")
	TArray<TObjectPtr<ACharacterTargetPoint>> OuterTargetPoints;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	int32 NumberOfTargetPoints = 9;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float InnerTargetPointDistance = 75.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float OuterTargetPointDistance = 150.f;

	UPROPERTY()
	TObjectPtr<USlashOverlay> SlashOverlay;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> FoliageSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AFoliageManager> FoliageManager;

	UPROPERTY(EditAnywhere, Category = "Death Screen")
	float DeathScreenDuration = 6.f;

	UPROPERTY(EditAnywhere, Category = "Death Screen")
	float DeathScreenSoundDelay = 1.25f;

	UPROPERTY(EditAnywhere, Category = "Death Screen")
	TObjectPtr<USoundBase> DeathScreenSound;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE TArray<TObjectPtr<ACharacterTargetPoint>> GetInnerTargetPoints() const { return InnerTargetPoints; }
	FORCEINLINE TArray<TObjectPtr<ACharacterTargetPoint>> GetOuterTargetPoints() const { return OuterTargetPoints; }
};
