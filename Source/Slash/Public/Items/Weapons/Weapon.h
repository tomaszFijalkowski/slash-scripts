// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class ABaseCharacter;
class UWeaponModifier;
class UBoxComponent;

UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();
	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator,
	           bool bPlayEquipSound = true);
	void AttachMeshToSocket(USceneComponent* InParent, FName InSocketName);
	void SetWeaponBoxCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	void EmptyActorsToIgnore();

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                  const FHitResult& SweepResult) override;
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                       const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

private:
	void DisableSphereCollision();
	void PlayEquipSound(bool bPlayEquipSound);
	void DeactivateEmbers();

	bool ActorIsSameType(const AActor* OtherActor);
	void BoxTrace(FHitResult& BoxHit);
	void ExecuteGetHit(const FHitResult& BoxHit);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<UWeaponModifier> ModifierClass;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<USoundBase> EquipSound;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<UParticleSystem> TrailEffect;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> WeaponBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> BoxTraceStart;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> BoxTraceEnd;

	UPROPERTY()
	TObjectPtr<UWeaponModifier> ModifierInstance;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActorsToIgnore;

public:
	FORCEINLINE TObjectPtr<UParticleSystem> GetTrailEffect() const { return TrailEffect; }
	FORCEINLINE TObjectPtr<UWeaponModifier> GetModifier() const { return ModifierInstance; }
};
