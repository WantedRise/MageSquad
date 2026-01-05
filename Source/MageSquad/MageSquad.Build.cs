// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MageSquad : ModuleRules
{
	public MageSquad(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

        // ������Ʈ ���� ���� ��� �߰�
        PublicIncludePaths.Add("MageSquad");

        // ��� �߰�
        PublicDependencyModuleNames.AddRange(new string[] {
            "AIModule",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "UMG",
            "Slate",
            "SlateCore",
            "GameplayTags",
            "GameplayAbilities",
            "GameplayTasks",
            "AnimGraphRuntime",
            "NavigationSystem",
            "Niagara",
            "MovieScene",
            "MovieSceneTracks",
            "LevelSequence",
            "CommonUI",
            "CommonInput",
            "PreLoadScreen",
            "Paper2D",
            "OnlineSubsystem",
            "OnlineSubsystemSteam",
            "SignificanceManager"
        });
    }
}
