// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StillHereIstanbul : ModuleRules
{
	public StillHereIstanbul(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"StillHereIstanbul",
			"StillHereIstanbul/Variant_Platforming",
			"StillHereIstanbul/Variant_Combat",
			"StillHereIstanbul/Variant_Combat/AI",
			"StillHereIstanbul/Variant_SideScrolling",
			"StillHereIstanbul/Variant_SideScrolling/Gameplay",
			"StillHereIstanbul/Variant_SideScrolling/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
