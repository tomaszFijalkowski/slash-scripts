// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FadeOverlay.generated.h"

class UImage;

UCLASS()
class SLASH_API UFadeOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	void PlayFadeInAnimation();
	void PlayFadeOutAnimation();
	float GetFadeOutEndTime();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FadeImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeOutAnimation;
};
