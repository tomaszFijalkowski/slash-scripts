// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Soul.h"
#include "Interfaces/PickupInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void ASoul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DriftDownward(DeltaTime);
}

void ASoul::BeginPlay()
{
	Super::BeginPlay();

	TraceTargetLocationZ();
}

void ASoul::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                 const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->PickupSoul(this);
		SpawnPickupEffect();
		PlayPickupSound();
		FinishEmissionAndDestroy();
	}
}

void ASoul::DriftDownward(float DeltaTime)
{
	const double LocationZ = GetActorLocation().Z;
	if (LocationZ > TargetLocationZ)
	{
		const FVector DeltaLocation = FVector(0.f, 0.f, -(DriftRate * DeltaTime));
		AddActorWorldOffset(DeltaLocation);
	}
}

void ASoul::TraceTargetLocationZ()
{
	// Trace for 2000 units until hits ground
	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, 2000.f);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	FHitResult HitResult;

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ObjectTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResult,
		true
	);

	const double DistanceFromGround = 100.f;
	TargetLocationZ = HitResult.ImpactPoint.Z + DistanceFromGround;
}
