// Fill out your copyright notice in the Description page of Project Settings.

#include "Interfaces/PickupInterface.h"

void IPickupInterface::SetOverlappingItem(AItem* Item)
{
}

void IPickupInterface::PickupSoul(ASoul* Soul)
{
}

void IPickupInterface::PickupTreasure(ATreasure* Treasure)
{
}

bool IPickupInterface::CanUseHealthPotion()
{
	return false;
}

void IPickupInterface::PickupHealthPotion(AHealthPotion* HealthPotion)
{
}

bool IPickupInterface::CanUseStaminaPotion()
{
	return false;
}

void IPickupInterface::PickupStaminaPotion(AStaminaPotion* StaminaPotion)
{
}
