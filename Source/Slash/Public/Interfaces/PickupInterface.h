// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

class AStaminaPotion;
class AHealthPotion;
class AItem;
class ASoul;
class ATreasure;

UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

class SLASH_API IPickupInterface
{
	GENERATED_BODY()

public:
	virtual void SetOverlappingItem(AItem* Item);
	virtual void PickupSoul(ASoul* Soul);
	virtual void PickupTreasure(ATreasure* Treasure);
	virtual bool CanUseHealthPotion();
	virtual void PickupHealthPotion(AHealthPotion* HealthPotion);
	virtual bool CanUseStaminaPotion();
	virtual void PickupStaminaPotion(AStaminaPotion* StaminaPotion);
};
