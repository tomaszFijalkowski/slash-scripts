// Fill out your copyright notice in the Description page of Project Settings.

#include "../../Public/Breakables/BreakableActor.h"
#include "NavModifierComponent.h"
#include "Characters/SlashCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Items/Treasure.h"
#include "Kismet/GameplayStatics.h"

ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	GeometryCollection->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GeometryCollection->SetGenerateOverlapEvents(true);
	SetRootComponent(GeometryCollection);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();

	if (GeometryCollection)
	{
		GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::OnBreak);
	}

	if (Sphere)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &ABreakableActor::OnSphereBeginOverlap);
		Sphere->OnComponentEndOverlap.AddDynamic(this, &ABreakableActor::OnSphereEndOverlap);
	}
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABreakableActor::OnBreak(const FChaosBreakEvent& BreakEvent)
{
	if (bBroken)
	{
		return;
	}
	bBroken = true;

	RemoveNavModifier();
	DisableCollision();
	PlayBreakSound();
	SpawnTreasure();
	SetLifeSpan(DestroyLifeSpan);
}

void ABreakableActor::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                           const FHitResult& SweepResult)
{
	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor))
	{
		SlashCharacter->AddOverlappingLowBreakable(this);
	}
}

void ABreakableActor::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor))
	{
		SlashCharacter->RemoveOverlappingLowBreakable(this);
	}
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, ABaseCharacter* Hitter)
{
	GeometryCollection->SetSimulatePhysics(true);
}

void ABreakableActor::SetFoliageProperties(UFoliageInstancedStaticMeshComponent* InstancedMesh,
                                           const FTransform& Transform, const FGuid Identifier)
{
	FoliageInstancedMesh = InstancedMesh;
	FoliageTransform = Transform;
	FoliageIdentifier = Identifier;
}

void ABreakableActor::RemoveNavModifier()
{
	if (UNavModifierComponent* NavModifier = FindComponentByClass<UNavModifierComponent>())
	{
		NavModifier->DestroyComponent();
	}
}

void ABreakableActor::DisableCollision()
{
	GeometryCollection->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GeometryCollection->OnActorEnableCollisionChanged();

	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->OnActorEnableCollisionChanged();

	Sphere->SetGenerateOverlapEvents(false);
}

void ABreakableActor::PlayBreakSound()
{
	if (BreakSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSound, GetActorLocation());
	}
}

void ABreakableActor::SpawnTreasure()
{
	TArray<TSubclassOf<AItem>> TreasureClassesToSpawn = TreasureClasses;

	if (WeaponClasses.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, WeaponClasses.Num() - 1);
		const TSubclassOf<AItem> SelectedTreasure = WeaponClasses[Selection];

		// Add a double weapon roll to the possible loot
		TreasureClassesToSpawn.Append({SelectedTreasure, SelectedTreasure});
	}

	UWorld* World = GetWorld();
	if (World && TreasureClassesToSpawn.Num() > 0)
	{
		FVector Location = GeometryCollection->GetComponentLocation();
		Location.Z += 50.f; // Offset on Z axis

		const int32 Selection = FMath::RandRange(0, TreasureClassesToSpawn.Num() - 1);
		const TSubclassOf<AItem> SelectedTreasure = TreasureClassesToSpawn[Selection];
		if (SelectedTreasure)
		{
			World->SpawnActor<AItem>(SelectedTreasure, Location, GetActorRotation());
		}
	}
}
