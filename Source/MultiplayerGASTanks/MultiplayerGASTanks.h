// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{														  // uint8:
	None				UMETA(DisplayName = "None"),      // 0
	Confirm				UMETA(DisplayName = "Confirm"),	  // 1
	Cancel				UMETA(DisplayName = "Cancel"),	  // 2
	Shoot				UMETA(DisplayName = "Shoot"),	  // 3
	Shield				UMETA(DisplayName = "Shield"),	  // 4
	Repair				UMETA(DisplayName = "Repair"),	  // 5
	Dash				UMETA(DisplayName = "Dash"),	  // 6
};