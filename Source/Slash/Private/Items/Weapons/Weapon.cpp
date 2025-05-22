// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Weapons/Weapon.h"
#include "NiagaraComponent.h"
#include "Characters/SlashCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Constants/GameplayTag.h"
#include "Interfaces/HitInterface.h"
#include "Items/Weapons/Modifiers/WeaponModifier.h"
#include "Kismet/GameplayStatics.h"

class ASlashCharacter;

AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponBox->SetUseCCD(true);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());

	ActorsToIgnore = TArray<AActor*>();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxBeginOverlap);

	if (ModifierClass)
	{
		ModifierInstance = NewObject<UWeaponModifier>(this, ModifierClass);
	}
}

void AWeapon::Equip(USceneComponent* InParent, const FName InSocketName, AActor* NewOwner, APawn* NewInstigator,
                    const bool bPlayEquipSound)
{
	ItemState = EItemState::EIS_Equipped;

	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	DisableSphereCollision();
	PlayEquipSound(bPlayEquipSound);
	DeactivateEmbers();
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName InSocketName)
{
	const FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::SetWeaponBoxCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(CollisionEnabled);
	}
}

void AWeapon::EmptyActorsToIgnore()
{
	if (ActorsToIgnore.Num() > 0)
	{
		ActorsToIgnore.Empty();
	}
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                   const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor))
	{
		SlashCharacter->SetOverlappingItem(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor))
	{
		SlashCharacter->SetOverlappingItem(nullptr);
	}
}

void AWeapon::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                const FHitResult& SweepResult)
{
	// Prevents the owner from hitting himself or his allies
	if (ActorIsSameType(OtherActor))
	{
		return;
	}

	FHitResult BoxHit;
	BoxTrace(BoxHit);

	if (AActor* DamagedActor = BoxHit.GetActor())
	{
		if (ActorIsSameType(DamagedActor))
		{
			return;
		}

		UGameplayStatics::ApplyDamage(
			DamagedActor,
			Damage,
			GetInstigator()->GetController(),
			this,
			UDamageType::StaticClass()
		);

		ExecuteGetHit(BoxHit);
		CreateFields(BoxHit.ImpactPoint);
	}
}

void AWeapon::DisableSphereCollision()
{
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::PlayEquipSound(const bool bPlayEquipSound)
{
	if (EquipSound && bPlayEquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
}

void AWeapon::DeactivateEmbers()
{
	if (ItemEffect)
	{
		ItemEffect->Deactivate();
	}
}

bool AWeapon::ActorIsSameType(const AActor* OtherActor)
{
	const bool bBothActorsAreEqual = OtherActor == GetOwner();
	const bool bBothActorsAreEnemies =
		OtherActor->ActorHasTag(GameplayTag::Enemy) &&
		GetOwner()->ActorHasTag(GameplayTag::Enemy);

	return bBothActorsAreEqual || bBothActorsAreEnemies;
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());

	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		BoxTraceExtent,
		BoxTraceStart->GetComponentRotation(),
		TraceTypeQuery1,
		false,
		ActorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit,
		true
	);

	if (BoxHit.bBlockingHit)
	{
		ActorsToIgnore.AddUnique(BoxHit.GetActor());
	}
}

void AWeapon::ExecuteGetHit(const FHitResult& BoxHit)
{
	if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()))
	{
		// implementation without UFUNCTION(BlueprintNativeEvent)
		// HitInterface->GetHit(BoxHit.ImpactPoint);

		// implementation with UFUNCTION(BlueprintNativeEvent) - Execute_GetHit is static here (IHitInterface)
		ABaseCharacter* WeaponOwner = Cast<ABaseCharacter>(GetOwner());
		HitInterface->IHitInterface::Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, WeaponOwner);
	}
}
