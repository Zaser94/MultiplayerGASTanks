// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MultiplayerGASTanks : ModuleRules
{
	public MultiplayerGASTanks(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreOnline", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "NavigationSystem", "OnlineSubsystem", "OnlineSubsystemEOS", "OnlineSubsystemUtils" });
		
		PrivateDependencyModuleNames.AddRange(new string[] {
		"GameplayAbilities",
		"GameplayTags",
		"GameplayTasks"});
	}
}
