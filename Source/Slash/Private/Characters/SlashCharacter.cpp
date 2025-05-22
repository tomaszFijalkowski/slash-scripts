// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/SlashCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GroomComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/CharacterTargetPoint.h"
#include "Components/AttributeComponent.h"
#include "Components/SphereComponent.h"
#include "Constants/Animation.h"
#include "Constants/GameplayTag.h"
#include "Constants/Socket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/SlashGameMode.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Items/HealthPotion.h"
#include "Items/Soul.h"
#include "Items/StaminaPotion.h"
#include "Items/Treasure.h"
#include "Items/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/FoliageManager.h"
#include "Slash/DebugMacros.h"

class AEnemy;

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	CurrentSpeed = UnequippedWalkSpeed;
	UpdateMaxWalkSpeed();

	// Set collision to enable enemy being able to hit us with a weapon
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = true;

	// Middle of the zoom distance range
	TargetZoomDistance = (MinZoomDistance + MaxZoomDistance) / 2;
	SpringArm->TargetArmLength = TargetZoomDistance;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	HairGroom = CreateDefaultSubobject<UGroomComponent>(TEXT("HairGroom"));
	HairGroom->SetupAttachment(GetMesh());
	HairGroom->AttachmentName = FString("head");

	EyebrowsGroom = CreateDefaultSubobject<UGroomComponent>(TEXT("EyebrowsGroom"));
	EyebrowsGroom->SetupAttachment(GetMesh());
	EyebrowsGroom->AttachmentName = FString("head");

	FoliageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("FoliageSphere"));
	FoliageSphere->SetupAttachment(GetRootComponent());

	// Interact only with WorldStatic objects (Foliage)
	FoliageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FoliageSphere->SetCollisionObjectType(ECC_WorldDynamic);
	FoliageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	FoliageSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);

	OverlappingLowBreakables = TArray<ABreakableActor*>();
	OuterTargetPoints = TArray<ACharacterTargetPoint*>();

	// AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetTargetZoomDistance(DeltaTime);

	if (bShouldRegenerateHealth)
	{
		RegenerateHealth(DeltaTime);
	}
	RegenerateStamina(DeltaTime);

	UpdateAllTargetPointPositions();
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Equip);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
		EnhancedInputComponent->BindAction(ScrollAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Scroll);
		EnhancedInputComponent->BindAction(ExitGameAction, ETriggerEvent::Triggered, this, &ASlashCharacter::ExitGame);
	}
}

void ASlashCharacter::VampiricHeal(const float HealthAmount)
{
	Super::VampiricHeal(HealthAmount);

	if (SlashOverlay)
	{
		SetHUDHealthBar();
	}
}

float ASlashCharacter::TakeDamage(const float DamageAmount, const FDamageEvent& DamageEvent,
                                  AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDodging())
	{
		return 0.f;
	}

	HandleDamage(DamageAmount);
	SetHUDHealthBar();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, ABaseCharacter* Hitter)
{
	if (IsDodging())
	{
		return;
	}

	Super::GetHit_Implementation(ImpactPoint, Hitter);

	if (Attributes && Attributes->IsAlive())
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::PickupSoul(ASoul* Soul)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::PickupTreasure(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

bool ASlashCharacter::CanUseHealthPotion()
{
	return Attributes ? Attributes->GetHealthPercent() < 1.f : false;
}

void ASlashCharacter::PickupHealthPotion(AHealthPotion* HealthPotion)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->Heal(HealthPotion->GetHealthAmount());
		const float Multiplier = HealthPotion->GetHealthRateMultiplier();
		const float Duration = HealthPotion->GetHealthRegenDuration();
		BoostHealthRegenRate(Multiplier, Duration);
		SetHUDHealthBar();
	}
}

bool ASlashCharacter::CanUseStaminaPotion()
{
	return Attributes ? Attributes->GetStaminaPercent() < 1.f : false;
}

void ASlashCharacter::PickupStaminaPotion(AStaminaPotion* StaminaPotion)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->RecoverStamina(StaminaPotion->GetStaminaAmount());
		const float RegenRateMultiplier = StaminaPotion->GetStaminaRateMultiplier();
		const float WalkSpeedMultiplier = StaminaPotion->GetWalkSpeedMultiplier();
		const float AnimationRateScale = StaminaPotion->GetAnimationRateScale();
		const float Duration = StaminaPotion->GetStaminaRegenDuration();
		BoostStamina(RegenRateMultiplier, WalkSpeedMultiplier, AnimationRateScale, Duration);
		SetHUDStaminaBar();
	}
}

void ASlashCharacter::AddOverlappingLowBreakable(ABreakableActor* Breakable)
{
	OverlappingLowBreakables.AddUnique(Breakable);
}

void ASlashCharacter::RemoveOverlappingLowBreakable(ABreakableActor* Breakable)
{
	OverlappingLowBreakables.Remove(Breakable);
}

void ASlashCharacter::AddOverlappingEnemy(AEnemy* Enemy)
{
	OverlappingEnemies.AddUnique(Enemy);
}

void ASlashCharacter::RemoveOverlappingEnemy(AEnemy* Enemy)
{
	OverlappingEnemies.Remove(Enemy);
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(GameplayTag::EngageableTarget);

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		InitializeSlashOverlay(PlayerController);
		InitializeEnhancedInput(PlayerController);
	}

	SpawnAndLinkTargetPoints();

	FoliageSphere->OnComponentBeginOverlap.AddDynamic(this, &ASlashCharacter::OnFoliageSphereBeginOverlap);
	FoliageSphere->OnComponentEndOverlap.AddDynamic(this, &ASlashCharacter::OnFoliageSphereEndOverlap);
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (!IsUnoccupied())
	{
		return;
	}

	const FVector2d MovementVector = Value.Get<FVector2d>();

	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2d LookAxisVector = Value.Get<FVector2d>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASlashCharacter::CustomJump(const FInputActionValue& Value)
{
	if (!IsUnoccupied())
	{
		return;
	}

	Jump();
}

void ASlashCharacter::Equip(const FInputActionValue& Value)
{
	if (AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem))
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
		}
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::Attack(const FInputActionValue& Value)
{
	if (CanAttack())
	{
		const bool bShouldUseLowerAttack = IsCloseToLowBreakable() && !IsCloseToEnemy();
		PlayAttackMontage(bShouldUseLowerAttack ? Section::SlashLowerAttack : NAME_None);
		ActionState = EActionState::EAS_Attacking;

		if (Attributes && SlashOverlay)
		{
			Attributes->ConsumeStamina(Attributes->GetAttackStaminaCost());
			SetHUDStaminaBar();
		}
	}
}

void ASlashCharacter::Dodge(const FInputActionValue& Value)
{
	if (CanDodge())
	{
		PlayDodgeMontage();
		ActionState = EActionState::EAS_Dodge;

		if (Attributes && SlashOverlay)
		{
			Attributes->ConsumeStamina(Attributes->GetDodgeStaminaCost());
			SetHUDStaminaBar();
		}
	}
}

void ASlashCharacter::Scroll(const FInputActionValue& Value)
{
	const float ZoomStep = (MaxZoomDistance - MinZoomDistance) / 2; // Allows for 3 equal steps
	const float ZoomDistance = TargetZoomDistance + Value.Get<float>() * ZoomStep;
	TargetZoomDistance = FMath::Clamp(ZoomDistance, MinZoomDistance, MaxZoomDistance);
}

void ASlashCharacter::ExitGame(const FInputActionValue& Value)
{
	if (ASlashGameMode* GameMode = Cast<ASlashGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		GameMode->ExitGame(PlayerController);
	}
}

void ASlashCharacter::EquipWeapon(AWeapon* OverlappingWeapon)
{
	OverlappingWeapon->Equip(GetMesh(), Socket::Weapon, this, this);
	CharacterState = ECharacterState::ECS_Equipped;
	EquippedWeapon = OverlappingWeapon;
	OverlappingItem = nullptr;
	CurrentSpeed = EquippedWalkSpeed;
	UpdateMaxWalkSpeed();
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(Montage::Unequip);
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(Montage::Equip);
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;

	ShowDeathScreen();
}

void ASlashCharacter::AttackEnd()
{
	Super::AttackEnd();

	if (Attributes && Attributes->IsAlive())
	{
		ActionState = EActionState::EAS_Unoccupied;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->EmptyActorsToIgnore();
	}
}

void ASlashCharacter::HitReactEnd()
{
	Super::HitReactEnd();

	if (Attributes && Attributes->IsAlive())
	{
		ActionState = EActionState::EAS_Unoccupied;
	}
}

void ASlashCharacter::DodgeEnd()
{
	if (Attributes && Attributes->IsAlive())
	{
		ActionState = EActionState::EAS_Unoccupied;
	}
}

bool ASlashCharacter::CanAttack()
{
	const bool bHasWeapon = CharacterState != ECharacterState::ECS_Unequipped;;
	const bool bHasEnoughStamina = Attributes->GetCurrentStamina() >= Attributes->
		GetAttackStaminaCost();

	return bHasWeapon &&
		bHasEnoughStamina &&
		IsUnoccupied() &&
		!GetCharacterMovement()->IsFalling();
}

bool ASlashCharacter::CanDodge()
{
	if (Attributes == nullptr)
	{
		return false;
	}

	const bool bHasEnoughStamina = Attributes->GetCurrentStamina() >= Attributes->GetDodgeStaminaCost();
	const bool bHasValidActionState = IsUnoccupied() || ActionState == EActionState::EAS_HitReaction;

	return bHasEnoughStamina &&
		bHasValidActionState &&
		!GetCharacterMovement()->IsFalling();
}

bool ASlashCharacter::CanDisarm()
{
	return IsUnoccupied() &&
		CharacterState != ECharacterState::ECS_Unequipped &&
		!GetCharacterMovement()->IsFalling();
}

bool ASlashCharacter::CanArm()
{
	return IsUnoccupied() &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		!GetCharacterMovement()->IsFalling() &&
		EquippedWeapon;
}

void ASlashCharacter::PlayDodgeMontage()
{
	DodgeMontage->RateScale = Attributes->GetAnimationRateScale();
	PlayMontageSection(DodgeMontage, Montage::Default);
}

void ASlashCharacter::PlayEquipMontage(const FName SectionName)
{
	EquipMontage->RateScale = Attributes->GetAnimationRateScale();
	PlayMontageSection(EquipMontage, SectionName);
}

void ASlashCharacter::EquipEnd()
{
	if (Attributes && Attributes->IsAlive())
	{
		ActionState = EActionState::EAS_Unoccupied;
	}
}

void ASlashCharacter::DisarmEnd()
{
	if (Attributes && !Attributes->IsAlive())
	{
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), Socket::Spine);
	}

	CharacterState = ECharacterState::ECS_Unequipped;
	CurrentSpeed = UnequippedWalkSpeed;
	UpdateMaxWalkSpeed();
}

void ASlashCharacter::ArmEnd()
{
	if (Attributes && !Attributes->IsAlive())
	{
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), Socket::Weapon);
	}

	CharacterState = ECharacterState::ECS_Equipped;
	CurrentSpeed = EquippedWalkSpeed;
	UpdateMaxWalkSpeed();
}

void ASlashCharacter::OnFoliageSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                  const FHitResult& SweepResult)
{
	SetFoliageManager();

	if (FoliageManager)
	{
		FoliageManager->HandleCloseFoliage(SweepResult);
	}
}

void ASlashCharacter::OnFoliageSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	SetFoliageManager();

	if (FoliageManager)
	{
		FoliageManager->HandleFarFoliage(OtherActor);
	}
}

bool ASlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::IsDodging()
{
	return ActionState == EActionState::EAS_Dodge;
}

bool ASlashCharacter::IsCloseToLowBreakable()
{
	return OverlappingLowBreakables.Num() > 0;
}

bool ASlashCharacter::IsCloseToEnemy()
{
	return OverlappingEnemies.Num() > 0;
}

void ASlashCharacter::InitializeSlashOverlay(const APlayerController* PlayerController)
{
	if (const ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD()))
	{
		SlashOverlay = SlashHUD->GetSlashOverlay();
		if (SlashOverlay && Attributes)
		{
			SetHUDHealthBar();
			SetHUDStaminaBar();
			SlashOverlay->SetSouls(0);
			SlashOverlay->SetGold(0);
		}
	}
}

void ASlashCharacter::InitializeEnhancedInput(const APlayerController* PlayerController)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(SlashMappingContext, 0);
	}
}

void ASlashCharacter::SetFoliageManager()
{
	if (FoliageManager == nullptr)
	{
		FoliageManager = Cast<AFoliageManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AFoliageManager::StaticClass()));
	}
}

void ASlashCharacter::SpawnAndLinkTargetPoints()
{
	SpawnTargetPoints(InnerTargetPoints, InnerTargetPointDistance);
	SpawnTargetPoints(OuterTargetPoints, OuterTargetPointDistance);
	LinkTargetPoints();
}

void ASlashCharacter::SpawnTargetPoints(TArray<TObjectPtr<ACharacterTargetPoint>>& TargetPoints,
                                        const float TargetPointDistance)
{
	if (UWorld* World = GetWorld())
	{
		for (int32 i = 0; i < NumberOfTargetPoints; i++)
		{
			FVector Location = CalculateTargetPointLocation(i, TargetPointDistance);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			ACharacterTargetPoint* SpawnedTargetPoint = World->SpawnActor<ACharacterTargetPoint>(
				ACharacterTargetPoint::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

			if (SpawnedTargetPoint)
			{
				SpawnedTargetPoint->SetActorHiddenInGame(true);
				SpawnedTargetPoint->SetActorEnableCollision(false);

				TargetPoints.Add(SpawnedTargetPoint);
			}
		}
	}
}

void ASlashCharacter::LinkTargetPoints()
{
	for (int32 i = 0; i < NumberOfTargetPoints; ++i)
	{
		ACharacterTargetPoint* InnerTargetPoint = InnerTargetPoints[i];
		ACharacterTargetPoint* OuterTargetPoint = OuterTargetPoints[i];

		if (InnerTargetPoint && OuterTargetPoint)
		{
			InnerTargetPoint->SetLinkedTargetPoint(OuterTargetPoint);
			OuterTargetPoint->SetLinkedTargetPoint(InnerTargetPoint);
		}
	}
}

void ASlashCharacter::UpdateAllTargetPointPositions()
{
	UpdateTargetPointPositions(InnerTargetPoints, InnerTargetPointDistance);
	UpdateTargetPointPositions(OuterTargetPoints, OuterTargetPointDistance);
}

void ASlashCharacter::UpdateTargetPointPositions(TArray<TObjectPtr<ACharacterTargetPoint>>& TargetPoints,
                                                 const float TargetPointDistance)
{
	if (TargetPoints.Num() > 0)
	{
		for (int32 i = 0; i < NumberOfTargetPoints; i++)
		{
			FVector Location = CalculateTargetPointLocation(i, TargetPointDistance);
			TargetPoints[i]->SetActorLocation(Location);
		}
	}
}

FVector ASlashCharacter::CalculateTargetPointLocation(const int32 Index, const float TargetPointDistance)
{
	const float AngleStep = 360.0f / NumberOfTargetPoints;
	const float Angle = FMath::DegreesToRadians(Index * AngleStep);
	const float OffsetX = FMath::Cos(Angle) * TargetPointDistance;
	const float OffsetY = FMath::Sin(Angle) * TargetPointDistance;
	const FVector Offset = FVector(OffsetX, OffsetY, 0.0f);
	return GetActorLocation() + Offset;
}

void ASlashCharacter::SetTargetZoomDistance(float DeltaTime)
{
	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		TargetZoomDistance,
		DeltaTime,
		ZoomSpeed
	);
}

void ASlashCharacter::SetHUDHealthBar()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::SetHUDHealthBoostIcon()
{
	if (SlashOverlay && Attributes)
	{
		const bool bIsVisible = Attributes->GetHealthRegenRateMultiplier() > 1.f;
		SlashOverlay->ToggleHealthRegenIcon(bIsVisible);
	}
}

void ASlashCharacter::RegenerateHealth(float DeltaTime)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenHealth(DeltaTime);
		SetHUDHealthBar();
	}
}

void ASlashCharacter::BoostHealthRegenRate(const float Multiplier, const float Duration)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->SetHealthRegenRateMultiplier(Multiplier);
		bShouldRegenerateHealth = true;
		SetHUDHealthBoostIcon();
		GetWorldTimerManager().SetTimer(
			HealthBoostRegenRateTimer,
			this,
			&ASlashCharacter::BoostHealthRegenRateEnd,
			Duration
		);
	}
}

void ASlashCharacter::BoostHealthRegenRateEnd()
{
	if (Attributes && SlashOverlay)
	{
		Attributes->SetHealthRegenRateMultiplier(1.f);
		bShouldRegenerateHealth = false;
		SetHUDHealthBoostIcon();
	}
}

void ASlashCharacter::SetHUDStaminaBar()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetStaminaPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::SetHUDStaminaBoostIcon()
{
	if (SlashOverlay && Attributes)
	{
		const bool bIsVisible = Attributes->GetStaminaRegenRateMultiplier() > 1.f;
		SlashOverlay->ToggleStaminaRegenIcon(bIsVisible);
	}
}

void ASlashCharacter::RegenerateStamina(float DeltaTime)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		SetHUDStaminaBar();
	}
}

void ASlashCharacter::BoostStamina(const float RegenRateMultiplier, const float WalkSpeedMultiplier,
                                   const float AnimationRateScale, const float Duration)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->SetStaminaRegenRateMultiplier(RegenRateMultiplier);
		Attributes->SetWalkSpeedMultiplier(WalkSpeedMultiplier);
		Attributes->SetAnimationRateScale(AnimationRateScale);
		UpdateMaxWalkSpeed();
		SetHUDStaminaBoostIcon();
		GetWorldTimerManager().SetTimer(
			StaminaBoostRegenRateTimer,
			this,
			&ASlashCharacter::BoostStaminaEnd,
			Duration
		);
	}
}

void ASlashCharacter::BoostStaminaEnd()
{
	if (Attributes && SlashOverlay)
	{
		Attributes->SetStaminaRegenRateMultiplier(1.f);
		Attributes->SetWalkSpeedMultiplier(1.f);
		Attributes->SetAnimationRateScale(1.f);
		UpdateMaxWalkSpeed();
		SetHUDStaminaBoostIcon();
	}
}

void ASlashCharacter::ShowDeathScreen()
{
	if (SlashOverlay)
	{
		SlashOverlay->PlayYouDiedAnimation();
	}

	GetWorldTimerManager().SetTimer(
		DeathScreenSoundTimer,
		this,
		&ASlashCharacter::PlayDeathScreenSound,
		DeathScreenSoundDelay
	);

	GetWorldTimerManager().SetTimer(
		RestartLevelTimer,
		this,
		&ASlashCharacter::RestartLevel,
		DeathScreenDuration
	);
}

void ASlashCharacter::PlayDeathScreenSound()
{
	if (DeathScreenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathScreenSound, GetActorLocation());
	}
}

void ASlashCharacter::RestartLevel()
{
	if (ASlashGameMode* GameMode = Cast<ASlashGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->RestartLevel();
	}
}
