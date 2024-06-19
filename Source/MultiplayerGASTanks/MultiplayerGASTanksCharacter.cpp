// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerGASTanksCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "GAS/TankAttributes.h"
#include "GAS/BaseAbilitySystemComponent.h"
#include "MultiplayerGASTanksGameMode.h"
#include "TankPlayerController.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMultiplayerGASTanksCharacter

AMultiplayerGASTanksCharacter::AMultiplayerGASTanksCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	TankBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TankBody"));
	TankBody->SetupAttachment(RootComponent);

	AbilitySystemComponent = CreateDefaultSubobject<UBaseAbilitySystemComponent>(TEXT("TankAbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	TankAttributes = CreateDefaultSubobject<UTankAttributes>(TEXT("TankAttributes"));

}

void AMultiplayerGASTanksCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(TankAttributes->GetHealthAttribute()).AddUObject(this, &AMultiplayerGASTanksCharacter::HealthChanged);
	// Si la vida maxima cambiase, hay que hacer otro GetGameplayAttributeValueChangeDelegate

	if (IsLocallyControlled())
	{
		// Logica para mostrar en pantalla el CD
		TArray<FGameplayTag> CooldownTagArray;
		CooldownTagArray.Add(FGameplayTag::RequestGameplayTag("Cooldown.Dash"));
		CooldownTagArray.Add(FGameplayTag::RequestGameplayTag("Cooldown.Repair"));
		CooldownTagArray.Add(FGameplayTag::RequestGameplayTag("Cooldown.Shield"));
		CooldownTagArray.Add(FGameplayTag::RequestGameplayTag("Cooldown.Shoot"));

		for (const FGameplayTag CooldownTag : CooldownTagArray)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Bindeando estado etiqueta: "+ CooldownTag.ToString());
			AbilitySystemComponent->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AMultiplayerGASTanksCharacter::CooldownTagChanged);
		}
	}

	Server_AskAboutMaxHealth();
}

void AMultiplayerGASTanksCharacter::Server_AskAboutMaxHealth_Implementation()
{
	Client_ReceiveMaxHealth(TankAttributes->GetMaxHealth());
}

void AMultiplayerGASTanksCharacter::Client_ReceiveMaxHealth_Implementation(float MaxHealthServerValue)
{
	MaxHealth_ServerValue = MaxHealthServerValue;
}

void AMultiplayerGASTanksCharacter::CooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		OnCooldownTagChanged.Broadcast(CooldownTag, false);
	}

	if (NewCount > 0)
	{
		OnCooldownTagChanged.Broadcast(CooldownTag, true);
	}
	
}

void AMultiplayerGASTanksCharacter::HealthChanged(const FOnAttributeChangeData& Data)
{
	if (IsLocallyControlled())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Vida actual: %f"), Data.NewValue));
		if (ATankPlayerController* controller = Cast<ATankPlayerController>(GetController()))
		{
			controller->UpdateHealthBar(Data.NewValue,MaxHealth_ServerValue);
		}
	}
	// Checking death
	if (Data.NewValue <= 0.0)
	{
		//AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("EffectsAppliedToSelf.Dead"));
		if (HasAuthority())
		{
			Multi_PlayerHasDie();
		}
		if(IsLocallyControlled())
		{
			FTimerHandle RespawnHandle;
			// Countdown before respawn
			GetWorldTimerManager().SetTimer(RespawnHandle, this, &AMultiplayerGASTanksCharacter::Server_RequestSpawn, RespawnTime, false);
		}
	}

}

void AMultiplayerGASTanksCharacter::HandleShoot(const FInputActionValue& Value)
{
	HandleGASAbility(Value.GetMagnitude() > 0, EAbilityInputID::Shoot);
}

void AMultiplayerGASTanksCharacter::HandleShield(const FInputActionValue& Value)
{
	HandleGASAbility(Value.GetMagnitude() > 0, EAbilityInputID::Shield);
}

void AMultiplayerGASTanksCharacter::HandleRepair(const FInputActionValue& Value)
{
	HandleGASAbility(Value.GetMagnitude() > 0, EAbilityInputID::Repair);
}

void AMultiplayerGASTanksCharacter::HandleDash(const FInputActionValue& Value)
{
	HandleGASAbility(Value.GetMagnitude() > 0, EAbilityInputID::Dash);
}

void AMultiplayerGASTanksCharacter::HandleGASAbility(const bool bIsKeyPressed, const EAbilityInputID AbilityInputID)
{

	if (AbilitySystemComponent)
	{
		if (bIsKeyPressed)
		{
			AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(AbilityInputID));
		}
		else
		{
			AbilitySystemComponent->AbilityLocalInputReleased(static_cast<int32>(AbilityInputID));
		}

	}
	
}

void AMultiplayerGASTanksCharacter::Multi_PlayerHasDie_Implementation()
{
	// Dead player lost the control, his character becomes invisible and with no collisions.
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (APlayerController* PContr = Cast<APlayerController>(GetController()))
	{
		DisableInput(PContr);
	}

}

void AMultiplayerGASTanksCharacter::Server_RequestSpawn_Implementation()
{
	//Random position
	FVector RandomLocation = { 0.0f, 0.0f, 0.0f };

	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation NavLocation;
		bool bFound = NavSys->GetRandomPointInNavigableRadius({ 1500,1700,0 }, 5000, NavLocation);
		if (bFound)
		{
			RandomLocation = NavLocation.Location;
		}
	}

	//AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("EffectsAppliedToSelf.Dead"));
	// How to instanciate Blueprint GE in c++:
	//UGameplayEffect* AttributesInitialization = NewObject<UGameplayEffect>(this, AttributesInitializationClass);

	AbilitySystemComponent->ApplyGameplayEffectToSelf(GetRespawnGameplayEffect(), 1, AbilitySystemComponent->MakeEffectContext());

	Multi_SpawnPlayer(RandomLocation);
}

void AMultiplayerGASTanksCharacter::Multi_SpawnPlayer_Implementation(FVector RandomLocation)
{
	//Player is visible and have collisions 
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// New Location
	SetActorLocation(RandomLocation);

	//Player is able to move again
	APlayerController* PContr = Cast<APlayerController>(GetController());
	EnableInput(PContr);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMultiplayerGASTanksCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiplayerGASTanksCharacter::Move);

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AMultiplayerGASTanksCharacter::HandleShoot);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AMultiplayerGASTanksCharacter::HandleShoot);

		EnhancedInputComponent->BindAction(ShieldAction, ETriggerEvent::Started, this, &AMultiplayerGASTanksCharacter::HandleShield);
		EnhancedInputComponent->BindAction(ShieldAction, ETriggerEvent::Completed, this, &AMultiplayerGASTanksCharacter::HandleShield);

		EnhancedInputComponent->BindAction(RepairAction, ETriggerEvent::Started, this, &AMultiplayerGASTanksCharacter::HandleRepair);
		EnhancedInputComponent->BindAction(RepairAction, ETriggerEvent::Completed, this, &AMultiplayerGASTanksCharacter::HandleRepair);

		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AMultiplayerGASTanksCharacter::HandleDash);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &AMultiplayerGASTanksCharacter::HandleDash);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Binding Inputs with GAS
	if (IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		FTopLevelAssetPath AbilityEnumAssetPath = FTopLevelAssetPath(FName("/Script/MultiplayerGASTanks"), FName("EAbilityInputID"));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"), AbilityEnumAssetPath, static_cast<int32>(EAbilityInputID::Confirm), static_cast<int32>(EAbilityInputID::Cancel)));

	}
}

void AMultiplayerGASTanksCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

UGameplayEffect* AMultiplayerGASTanksCharacter::GetRespawnGameplayEffect()
{
	if (!ResetAttributesForSpawn)
	{
		// Respawn Gameplay Effect. Just reseting Health using MaxHealth.
		ResetAttributesForSpawn = NewObject<UGameplayEffect>(GetTransientPackage(), FName("[Reset Attributes For Spawn]: " + this->GetName()));
		ResetAttributesForSpawn->DurationPolicy = EGameplayEffectDurationType::Instant;

		int32 Idx = ResetAttributesForSpawn->Modifiers.Num();
		ResetAttributesForSpawn->Modifiers.SetNum(Idx + 1);
		FGameplayModifierInfo& Strength = ResetAttributesForSpawn->Modifiers[Idx];
		Strength.ModifierMagnitude = FScalableFloat(TankAttributes->GetMaxHealth());
		Strength.ModifierOp = EGameplayModOp::Override;
		Strength.Attribute = UTankAttributes::GetHealthAttribute();
	}

	return ResetAttributesForSpawn;
}