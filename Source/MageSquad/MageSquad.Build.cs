// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MageSquad : ModuleRules
{
	public MageSquad(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

        // 프로젝트 공개 폴더 경로 추가
        PublicIncludePaths.Add("MageSquad");

        // 모듈 추가
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
            "OnlineSubsystemUtils"
        });
    }
}
