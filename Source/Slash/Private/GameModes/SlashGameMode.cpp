// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/SlashGameMode.h"
#include "HUD/FadeOverlay.h"
#include "HUD/SlashHUD.h"
#include "Kismet/GameplayStatics.h"

void ASlashGameMode::RestartLevel()
{
	if (UFadeOverlay* const FadeOverlay = GetFadeOverlay())
	{
		FadeOverlay->PlayFadeOutAnimation();
		GetWorldTimerManager().SetTimer(
			FadeOutTimer,
			this,
			&ASlashGameMode::RestartLevelAfterFadeOut,
			FadeOverlay->GetFadeOutEndTime()
		);
	}
}

void ASlashGameMode::ExitGame(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
	}
}

void ASlashGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UFadeOverlay* const FadeOverlay = GetFadeOverlay())
	{
		FadeOverlay->PlayFadeInAnimation();
	}
}

UFadeOverlay* ASlashGameMode::GetFadeOverlay()
{
	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (const ASlashHUD* HUD = Cast<ASlashHUD>(PlayerController->GetHUD()))
			{
				if (UFadeOverlay* FadeOverlay = HUD->GetFadeOverlay())
				{
					return FadeOverlay;
				}
			}
		}
	}

	return nullptr;
}

void ASlashGameMode::RestartLevelAfterFadeOut()
{
	if (UWorld* World = GetWorld())
	{
		World->FlushLevelStreaming();

		const FString CurrentLevelName = World->GetMapName();
		const FString ShortLevelName = CurrentLevelName.Replace(*World->StreamingLevelsPrefix, TEXT(""));
		UGameplayStatics::OpenLevel(this, FName(*ShortLevelName), true);
	}
}
