// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SlashHUD.generated.h"

class UFadeOverlay;
class USlashOverlay;

UCLASS()
class SLASH_API ASlashHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void PreInitializeComponents() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<USlashOverlay> SlashOverlayClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> FadeOverlayClass;

	UPROPERTY()
	TObjectPtr<USlashOverlay> SlashOverlay;

	UPROPERTY()
	TObjectPtr<UFadeOverlay> FadeOverlay;

public:
	FORCEINLINE TObjectPtr<USlashOverlay> GetSlashOverlay() const { return SlashOverlay; }
	FORCEINLINE TObjectPtr<UFadeOverlay> GetFadeOverlay() const { return FadeOverlay; }
};
