// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE_FruitMountain : ModuleRules
{
	public UE_FruitMountain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"UMG",          // UMG 모듈 추가
			"Slate",        // Slate 모듈 추가
			"SlateCore"     // SlateCore 모듈 추가
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
