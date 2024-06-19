// Fill out your copyright notice in the Description page of Project Settings.


#include "EOS/EOSGameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Online/OnlineSessionNames.h"
#include <Kismet/GameplayStatics.h>

DEFINE_LOG_CATEGORY(LogEOSGameInstance);

void UEOSGameInstance::Init()
{
	Super::Init();
}


void UEOSGameInstance::EOSLogin()
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface();

		if (Identity)
		{
			FOnlineAccountCredentials AccountDetails;
			AccountDetails.Id = "";
			AccountDetails.Token = "";
			AccountDetails.Type = "deviceID";

			Identity->OnLoginCompleteDelegates->AddUObject(this, &UEOSGameInstance::OnEOSLoginCompleted);
			Identity->Login(0, AccountDetails);
		}
	}
}

void UEOSGameInstance::OnEOSLoginCompleted(int32 LocalUserNum, bool bSuccess, const FUniqueNetId& UserID, const FString& ErrorDetails)
{
	if (bSuccess)
	{
		UE_LOG(LogEOSGameInstance, Display, TEXT("EOS LOGIN SUCCESS || UserID: %s"), *UserID.ToString());
		EOSLoginSucceed = true;
		OnEOSLoginSuccess.Broadcast();
	}
	else
	{
		UE_LOG(LogEOSGameInstance, Error, TEXT("Error: EOS LOGIN FAILURE!! Details: %s"), *ErrorDetails);
	}

}

void UEOSGameInstance::CreateEOSSession(int32 PublicConnections)
{
	//if (!EOSLoginSucceed)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "EOS LOGIN REQUIRED!");
	//	return;
	//}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem)
	{
		if (IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface())
		{
			FOnlineSessionSettings SessioInfo;
			SessioInfo.bIsDedicated = false;
			SessioInfo.bAllowInvites = true;
			SessioInfo.bIsLANMatch = false;
			SessioInfo.NumPublicConnections = PublicConnections;
			SessioInfo.bUseLobbiesIfAvailable = false;
			SessioInfo.bUsesPresence = false;
			SessioInfo.bShouldAdvertise = true;
			SessioInfo.Set(SEARCH_KEYWORDS, FString("TankSession", EOnlineDataAdvertisementType::ViaOnlineService));

			Session->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnCreateSessionCompleted);
			Session->CreateSession(0, FName("TankSession"), SessioInfo);
		}
	}
}

void UEOSGameInstance::OnCreateSessionCompleted(FName SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogEOSGameInstance, Display, TEXT("Session Created: %s"), *SessionName.ToString());
		GetWorld()->ServerTravel(MapURLHost);
	}

}

void UEOSGameInstance::FindEOSSession()
{

	//if (!EOSLoginSucceed)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "EOS LOGIN REQUIRED!");
	//	return;
	//}

	if (IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface())
		{
			SessionSearch = MakeShared<FOnlineSessionSearch>();
			//SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, false, EOnlineComparisonOp::Equals);
			SessionSearch->QuerySettings.SearchParams.Empty();
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 1;

			Session->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnFindEOSSessionCompleted);
			Session->FindSessions(0, SessionSearch.ToSharedRef());
		}

	}
}

void UEOSGameInstance::OnFindEOSSessionCompleted(bool bSuccess)
{
	if (bSuccess)
	{
		if (!SessionSearch->SearchResults.IsEmpty())
		{														//
		OnSessionFinded.Broadcast(SessionSearch->SearchResults[0].GetSessionIdStr()/*.Session.OwningUserId->ToString()*/);
		}
		else
		{
			UE_LOG(LogEOSGameInstance, Warning, TEXT("OnFindEOSSessionCompleted: SEARCH RESULT IS EMPTY!!"));
		}

	}
	else
	{
		UE_LOG(LogEOSGameInstance, Warning, TEXT("OnFindEOSSessionCompleted: NO SESSIONS FINDED!!"));
	}

}


void UEOSGameInstance::JoinEOSSession()
{

	//if (!EOSLoginSucceed)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "EOS LOGIN REQUIRED!");
	//	return;
	//}

	if (!SessionSearch->SearchResults.IsEmpty())
	{
		if (IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface())
			{
				Session->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSGameInstance::OnJoinEOSSessionCompleted);
				Session->JoinSession(0, FName("TankSession"), SessionSearch->SearchResults[0]);
			}
		}
	}
	else
	{
		UE_LOG(LogEOSGameInstance, Warning, TEXT("JoinEOSSession: SEARCH RESULT IS EMPTY!!"));
		OnJoinSessionFailure.Broadcast();
	}
		
}

void UEOSGameInstance::OnJoinEOSSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogEOSGameInstance, Display, TEXT("Join Session [%s] Success!"), *SessionName.ToString());
		if (IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineSessionPtr Session = OnlineSubsystem->GetSessionInterface())
			{
				FString JoinAddress;
				Session->GetResolvedConnectString(SessionName, JoinAddress);
				if (!JoinAddress.IsEmpty())
				{
					if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
					{
						UE_LOG(LogEOSGameInstance, Display, TEXT("Travelling to  %s"), *JoinAddress);
						PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
					}
				}
				else
				{
					UE_LOG(LogEOSGameInstance, Error, TEXT("Join Address is empty"));
					OnJoinSessionFailure.Broadcast();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogEOSGameInstance, Display, TEXT("Join Session is not Success: %s"), *JoinSessionResultToString(Result));
		OnJoinSessionFailure.Broadcast();
	}
}

FString UEOSGameInstance::JoinSessionResultToString(EOnJoinSessionCompleteResult::Type Result)
{
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success: return "Success";
	case EOnJoinSessionCompleteResult::SessionIsFull: return "SessionIsFull";
	case EOnJoinSessionCompleteResult::SessionDoesNotExist: return "SessionDoesNotExist";
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress: return "CouldNotRetrieveAddress";
	case EOnJoinSessionCompleteResult::AlreadyInSession: return "AlreadyInSession";
	case EOnJoinSessionCompleteResult::UnknownError: return "UnknownError";
	default: return "None";
	}
}
