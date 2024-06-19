// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BaseGameplayAbility.generated.h"

struct FGameplayAbilitySpecHandle;
struct FGameplayAbilityActorInfo;
struct FGameplayAbilityActivationInfo;
struct FGameplayEventData;
struct FGameplayAbilityTargetDataHandle;
struct FGameplayTag;
/**
 * 
 */
UCLASS()
class MULTIPLAYERGASTANKS_API UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
protected:

	//OVERRIDES
	void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Called by any client, inside ActivateAbility, when the server requires some info only available at that client
	UFUNCTION(BlueprintCallable)
	void ClientSendTargetData(const FTransform& TargetDataTransform, FGameplayTag ApplicationTag);

	void NotifyTargetDataReady(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	// Called the server, inside ActivateAbility, when the server requires some info only available at that client
	UFUNCTION(BlueprintCallable)
	void ServerWaitsForClientsInfo();

	// Server validates clients data before calling ActivateAbilityWithTargetData
	void ServerValidatesTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);

	// Al abilities that required send info to the server before execution, should implement here the logic of ActivateAbility
	UFUNCTION(BlueprintImplementableEvent)
	void ActivateAbilityWithTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);

	// Important: Set from editor
	UPROPERTY(EditAnywhere, Category="Cooldown")
	FGameplayTag CooldownTag;

	UPROPERTY(EditAnywhere, Category = "Cooldown")
	float CooldownTime;

	UFUNCTION(Client,Reliable,BlueprintCallable)
	void DisplayCDText(float CDTime,const FString &SkillName);
	void DisplayCDText_Implementation(float CDTime, const FString& SkillName);

private:
	bool bIsServerAbility = false;
	FDelegateHandle NotifyTargetDataReadyDelegateHandle;
	FTimerHandle CooldownHandle;

	//void StartAbilityCooldown();
	//UFUNCTION()
	//void EndAbilityCooldown();
};
