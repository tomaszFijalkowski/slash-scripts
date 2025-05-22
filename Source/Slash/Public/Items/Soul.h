// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Soul.generated.h"

class UNiagaraSystem;

UCLASS()
class SLASH_API ASoul : public AItem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                  const FHitResult& SweepResult) override;

private:
	void DriftDownward(float DeltaTime);
	void TraceTargetLocationZ();

	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	int32 Souls;

	UPROPERTY(EditAnywhere)
	float DriftRate = 100.f;

	double TargetLocationZ;

public:
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE void SetSouls(const int32 Amount) { Souls = Amount; }
};
