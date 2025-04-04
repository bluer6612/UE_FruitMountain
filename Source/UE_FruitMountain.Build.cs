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
			"AssetRegistry" // AssetRegistry 추가
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
