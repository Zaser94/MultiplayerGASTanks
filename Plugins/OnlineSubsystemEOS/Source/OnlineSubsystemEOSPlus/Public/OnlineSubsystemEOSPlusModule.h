// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Templates/UniquePtr.h"

/**
 * OSS that aggregates EOS plus the platform OSS
 */
class FOnlineSubsystemEOSPlusModule : public IModuleInterface
{
public:
	virtual ~FOnlineSubsystemEOSPlusModule() = default;

// IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}
// ~IModuleInterface

private:
	void LoadBaseOSS();

	/** Class responsible for creating instance(s) of the subsystem */
	TUniquePtr<class FOnlineFactoryEOSPlus> PlusFactory;
};
