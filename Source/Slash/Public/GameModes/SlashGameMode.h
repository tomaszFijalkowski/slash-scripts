// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SlashGameMode.generated.h"

class UFadeOverlay;

UCLASS()
class SLASH_API ASlashGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RestartLevel();

	UFUNCTION(BlueprintCallable)
	void ExitGame(APlayerController* PlayerController);

protected:
	virtual void BeginPlay() override;

private:
	UFadeOverlay* GetFadeOverlay();
	void RestartLevelAfterFadeOut();

	UPROPERTY()
	FTimerHandle FadeOutTimer;
};
