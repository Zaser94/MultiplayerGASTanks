// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "GAS/TankAttributes.h"
#include "MultiplayerGASTanks.h"
#include "MultiplayerGASTanksCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UGameplayEffect;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownChanged, FGameplayTag, CooldownTag, bool, bHasStarted);

UCLASS(config=Game)
class AMultiplayerGASTanksCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	// INPUTS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShieldAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RepairAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;


public:
	AMultiplayerGASTanksCharacter();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TankBody;

	//Interface implementation. Mandatory for GAS comunication between actors.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	UPROPERTY(BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly)
	UTankAttributes* TankAttributes;
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	// Function called on the server
	UFUNCTION(Server, Reliable)
	void Server_AskAboutMaxHealth();
	void Server_AskAboutMaxHealth_Implementation();

	// Client callback
	UFUNCTION(Client, Reliable)
	void Client_ReceiveMaxHealth(float MaxHealthServerValue);
	void Client_ReceiveMaxHealth_Implementation(float MaxHealthServerValue);

	UPROPERTY(EditAnywhere, Category ="Respawn")
	float RespawnTime = 5;

	//Blueprint Gameplay Effect.
	UPROPERTY(EditAnywhere, Category = "Respawn")
	TSubclassOf<UGameplayEffect> AttributesInitializationClass;

	virtual void CooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount);

public:
	UPROPERTY(BlueprintAssignable)
	FOnCooldownChanged OnCooldownTagChanged;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
	float MaxHealth_ServerValue = 0;
	// GAS input handling
	void HandleShoot(const FInputActionValue& Value);
	void HandleShield(const FInputActionValue& Value);
	void HandleRepair(const FInputActionValue& Value);
	void HandleDash(const FInputActionValue& Value);

	void HandleGASAbility(const bool bIsKeyPressed, const EAbilityInputID AbilityInputID);

	//The server replicate player's death over the net
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayerHasDie();
	void Multi_PlayerHasDie_Implementation();

	//After X seconds dead players will ask the server to be spawned
	UFUNCTION(Server, Reliable)
	void Server_RequestSpawn();
	void Server_RequestSpawn_Implementation();

	// The server spawns dead player and replicates it over the net
	UFUNCTION(NetMulticast, Reliable)
	void Multi_SpawnPlayer(FVector RandomLocation);
	void Multi_SpawnPlayer_Implementation(FVector RandomLocation);

	UGameplayEffect* GetRespawnGameplayEffect();
	UGameplayEffect* ResetAttributesForSpawn;

};

