// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HJ_Project : ModuleRules
{
    public HJ_Project(ReadOnlyTargetRules Target) : base(Target)
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
            "NavigationSystem",
            "UMG",
            "Slate"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        PublicIncludePaths.AddRange(new string[] {
            "HJ_Project",
            "HJ_Project/Variant_Platforming",
            "HJ_Project/Variant_Platforming/Animation",
            "HJ_Project/Variant_Combat",
            "HJ_Project/Variant_Combat/AI",
            "HJ_Project/Variant_Combat/Animation",
            "HJ_Project/Variant_Combat/Gameplay",
            "HJ_Project/Variant_Combat/Interfaces",
            "HJ_Project/Variant_Combat/UI",
            "HJ_Project/Variant_SideScrolling",
            "HJ_Project/Variant_SideScrolling/AI",
            "HJ_Project/Variant_SideScrolling/Gameplay",
            "HJ_Project/Variant_SideScrolling/Interfaces",
            "HJ_Project/Variant_SideScrolling/UI"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
