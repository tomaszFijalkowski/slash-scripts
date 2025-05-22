// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlashOverlay.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;

UCLASS()
class SLASH_API USlashOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthPercent(float Percent);
	void ToggleHealthRegenIcon(bool IsVisible);
	void SetStaminaPercent(float Percent);
	void ToggleStaminaRegenIcon(bool IsVisible);
	void SetSouls(int32 Souls);
	void SetGold(int32 Gold);
	void PlayYouDiedAnimation();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HealthRegenIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> StaminaRegenIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SoulsTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> YouDiedTextBlock;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> YouDiedAnimation;
};
