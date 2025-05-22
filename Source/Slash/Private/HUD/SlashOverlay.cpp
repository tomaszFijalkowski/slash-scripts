// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/SlashOverlay.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USlashOverlay::SetHealthPercent(const float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::ToggleHealthRegenIcon(const bool IsVisible)
{
	if (HealthRegenIcon)
	{
		HealthRegenIcon->SetVisibility(IsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USlashOverlay::SetStaminaPercent(const float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::ToggleStaminaRegenIcon(const bool IsVisible)
{
	if (StaminaRegenIcon)
	{
		StaminaRegenIcon->SetVisibility(IsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void USlashOverlay::SetSouls(const int32 Souls)
{
	if (SoulsTextBlock)
	{
		const FString SoulsString = FString::Printf(TEXT("%d"), Souls);
		const FText SoulsText = FText::FromString(SoulsString);
		SoulsTextBlock->SetText(SoulsText);
	}
}

void USlashOverlay::SetGold(const int32 Gold)
{
	if (GoldTextBlock)
	{
		const FString GoldString = FString::Printf(TEXT("%d"), Gold);
		const FText GoldText = FText::FromString(GoldString);
		GoldTextBlock->SetText(GoldText);
	}
}

void USlashOverlay::PlayYouDiedAnimation()
{
	if (YouDiedTextBlock && YouDiedAnimation)
	{
		PlayAnimation(YouDiedAnimation);
	}
}
