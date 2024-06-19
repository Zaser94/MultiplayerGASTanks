// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
	#include "Templates/SharedPointer.h"
#include "EOSGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFinded, FString, ServerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJoinSessionFailure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEOSLoginSuccess);

DECLARE_LOG_CATEGORY_EXTERN(LogEOSGameInstance, Log, All);

/**
 * 
 */
UCLASS()
class MULTIPLAYERGASTANKS_API UEOSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void Init() override;

	UPROPERTY(BlueprintAssignable)
	FOnEOSLoginSuccess OnEOSLoginSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnSessionFinded OnSessionFinded;

	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionFailure OnJoinSessionFailure;

protected:

	//Online functions require a login user. Every client would login as a TrustedServer (developer portal setting).
	UFUNCTION(BlueprintCallable)
	void EOSLogin();
	void OnEOSLoginCompleted(int32 LocalUserNum, bool bSuccess, const FUniqueNetId& UserID, const FString& ErrorDetails);

	UFUNCTION(BlueprintCallable)
	void CreateEOSSession(int32 PublicConnections);
	void OnCreateSessionCompleted(FName SessionName, bool bSuccess);

	UFUNCTION(BlueprintCallable)
	void FindEOSSession();
	void OnFindEOSSessionCompleted(bool bSuccess);

	// Join the first session finded after using FindEOSSession
	UFUNCTION(BlueprintCallable)
	void JoinEOSSession();

	void OnJoinEOSSessionCompleted( FName SessionName, EOnJoinSessionCompleteResult::Type Result);


	UPROPERTY(EditAnywhere)
	FString MapURLHost = "";

	// Configuring the search and storing the results:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

private:
	bool EOSLoginSucceed = false;
	//TODO: put this in a generic library function
	FString JoinSessionResultToString(EOnJoinSessionCompleteResult::Type Result);
};
