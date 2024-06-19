// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TankAttributes.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class MULTIPLAYERGASTANKS_API UTankAttributes : public UAttributeSet
{
	GENERATED_BODY()
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// Current health
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UTankAttributes, Health)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData DamageRecieved;
	ATTRIBUTE_ACCESSORS(UTankAttributes, DamageRecieved)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData HealingRecieved;
	ATTRIBUTE_ACCESSORS(UTankAttributes, HealingRecieved)

	UPROPERTY(BlueprintReadOnly/*, ReplicatedUsing = OnRep_MaxHealth*/)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UTankAttributes, MaxHealth)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData DashForceMultiplier;
	ATTRIBUTE_ACCESSORS(UTankAttributes, DashForceMultiplier)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData ShieldTime;
	ATTRIBUTE_ACCESSORS(UTankAttributes, ShieldTime)

	// REGENERATION SKILL
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData RegenHealthPerSecond;
	ATTRIBUTE_ACCESSORS(UTankAttributes, RegenHealthPerSecond)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData RegenTime;
	ATTRIBUTE_ACCESSORS(UTankAttributes, RegenTime)

	// SHOOT SKILL
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData BulletDamage;
	ATTRIBUTE_ACCESSORS(UTankAttributes, BulletDamage)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData ShootCooldownTime;
	ATTRIBUTE_ACCESSORS(UTankAttributes, ShootCooldownTime)

private:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	void ClampHealth();
//
//	UFUNCTION()
//	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
};
