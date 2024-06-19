// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/TankAttributes.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

void UTankAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UTankAttributes, Health, COND_None, REPNOTIFY_Always);
	//DOREPLIFETIME_CONDITION_NOTIFY(UTankAttributes, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UTankAttributes::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageRecievedAttribute())
	{
		// Clamping health between its limits
		SetHealth(GetHealth() - GetDamageRecieved());
		ClampHealth();

		// Meta attributes must reset after each use
		SetDamageRecieved(0);
	}
	else if(Data.EvaluatedData.Attribute == GetHealingRecievedAttribute())
	{
		// Clamping health between its limits
		SetHealth(GetHealth() + GetHealingRecieved());
		ClampHealth();

		// Meta attributes must reset after each use
		SetHealingRecieved(0);
	}
}

void UTankAttributes::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTankAttributes, Health, OldHealth);
}

void UTankAttributes::ClampHealth()
{
	if (GetHealth() < 0.0 || GetHealth() > GetMaxHealth())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}

//void UTankAttributes::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
//{
//	GAMEPLAYATTRIBUTE_REPNOTIFY(UTankAttributes, MaxHealth, OldMaxHealth);
//}
