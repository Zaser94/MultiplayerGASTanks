// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/BaseGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "MultiplayerGASTanks/MultiplayerGASTanksCharacter.h"

void UBaseGameplayAbility::ServerValidatesTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag)
{
	bIsServerAbility = true;

	// retrieve data
	const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(0);
	if (!TargetData)
	{
		// client sent us bad data
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// This method replace the usual ActivateAbility
	ActivateAbilityWithTargetData(TargetDataHandle, ApplicationTag);

}

void UBaseGameplayAbility::DisplayCDText_Implementation(float CDTime, const FString& SkillName)
{
	GEngine->AddOnScreenDebugMessage(-1, CDTime, FColor::Red,"["+ SkillName+" COOLDOWN]");
}

void UBaseGameplayAbility::ClientSendTargetData(const FTransform& TargetDataTransform, FGameplayTag ApplicationTag)
{
	// Format the TargetData for GAS RPC

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_LocationInfo* TargetData = new FGameplayAbilityTargetData_LocationInfo(); //** USE OF new() IS **REQUIRED** **

	TargetData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	TargetData->TargetLocation.LiteralTransform = FTransform(TargetDataTransform.GetLocation());

	TargetDataHandle.Add(TargetData);

	// Notify self (local client) *AND* server that TargetData is ready to be processed
	NotifyTargetDataReady(TargetDataHandle, FGameplayTag());  // send with a gameplay tag, or empty
}

void UBaseGameplayAbility::NotifyTargetDataReady(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	// [xist] is this (from Lyra) like an "if is handle valid?" check? seems so, keeping it as such.
	if (!ASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false);  // do not replicate
		return;
	}

	// if commit fails, cancel ability
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);  // replicate cancellation
		return;
	}

	// true if we need to replicate this target data to the server
	const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();

	// Start a scoped prediction window
	FScopedPredictionWindow	ScopedPrediction(ASC);

	// Lyra does this memcopy operation; const cast paranoia is real. We'll keep it.
	// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
	const FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

	// if this isn't the local player on the server, then notify the server
	if (bShouldNotifyServer)
		ASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, ASC->ScopedPredictionKey);

	// Execute the ability we've now successfully committed
	ServerValidatesTargetData(LocalTargetDataHandle, ApplicationTag);

	// We've processed the data, clear it from the RPC buffer
	ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UBaseGameplayAbility::ServerWaitsForClientsInfo()
{
	if (UAbilitySystemComponent* ASC = GetCurrentActorInfo()->AbilitySystemComponent.Get())
	{
		// plug into AbilitySystemComponent to receive Target Data from client
		NotifyTargetDataReadyDelegateHandle = ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UBaseGameplayAbility::NotifyTargetDataReady);
	}
}


void UBaseGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsServerAbility)
	{
		if (UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get())
		{
			// Clean up the Ability Component after a server ability execution
			ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(NotifyTargetDataReadyDelegateHandle);
			ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}