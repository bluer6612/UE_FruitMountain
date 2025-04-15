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
			"UMG", 
			"Slate", 
			"SlateCore", 
			"AssetRegistry"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// 에디터 모듈 의존성 추가
		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd"
				}
			);
		}
	}
}
