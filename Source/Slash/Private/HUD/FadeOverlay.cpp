// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/FadeOverlay.h"
#include "Animation/WidgetAnimation.h"

void UFadeOverlay::PlayFadeInAnimation()
{
	if (FadeImage && FadeInAnimation)
	{
		PlayAnimation(FadeInAnimation);
	}
}

void UFadeOverlay::PlayFadeOutAnimation()
{
	if (FadeImage && FadeOutAnimation)
	{
		PlayAnimation(FadeOutAnimation);
	}
}

float UFadeOverlay::GetFadeOutEndTime()
{
	return FadeOutAnimation ? FadeOutAnimation->GetEndTime() : 0.f;
}
