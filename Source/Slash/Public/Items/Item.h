// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class UNiagaraSystem;
class USphereComponent;
class UNiagaraComponent;

enum class EItemState : uint8
{
	EIS_Hovering,
	EIS_Equipped
};

UCLASS()
class SLASH_API AItem : public AActor
{
	GENERATED_BODY()

public:
	AItem();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void FinishEmissionAndDestroy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Sine Parameters")
	float Amplitude = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Sine Parameters")
	float TimeConstant = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Sine Parameters")
	float RotationSpeed = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	EItemState ItemState = EItemState::EIS_Hovering;

	UFUNCTION(BlueprintPure)
	float TransformedSin();

	UFUNCTION(BlueprintPure)
	float TransformedCos();

	template <typename T>
	T Avg(T First, T Second);

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                  const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void SpawnPickupEffect();
	virtual void PlayPickupSound();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraComponent> ItemEffect;

	UPROPERTY(EditAnywhere)
	float DestroyDelay = 1.f;

private:
	void DisableMesh();
	void DisableSphere();
	void DestroySelf();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunningTime;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> PickupEffect;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY()
	FTimerHandle DestroyTimer;
};

template <typename T>
T AItem::Avg(T First, T Second)
{
	return (First + Second) / 2;
}
