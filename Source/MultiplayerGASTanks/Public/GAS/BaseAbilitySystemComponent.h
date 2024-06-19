// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BaseAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERGASTANKS_API UBaseAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	// Input bound to an ability is pressed
	virtual void AbilityLocalInputPressed(int32 InputID) override;
	
};
