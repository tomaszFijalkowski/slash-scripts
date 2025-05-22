// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/SlashHUD.h"
#include "Blueprint/UserWidget.h"
#include "HUD/FadeOverlay.h"
#include "HUD/SlashOverlay.h"

void ASlashHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* Controller = World->GetFirstPlayerController())
		{
			if (SlashOverlayClass)
			{
				SlashOverlay = CreateWidget<USlashOverlay>(Controller, SlashOverlayClass);
				SlashOverlay->AddToViewport();
			}

			if (FadeOverlayClass)
			{
				FadeOverlay = CreateWidget<UFadeOverlay>(Controller, FadeOverlayClass);
				FadeOverlay->AddToViewport();
			}
		}
	}
}
