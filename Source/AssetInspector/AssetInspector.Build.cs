// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AssetInspector : ModuleRules
{
	public AssetInspector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		if (Target.bBuildEditor)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
			);
				
		
			PrivateIncludePaths.AddRange(
				new string[] {
					// ... add other private include paths required here ...
				}
			);
			
		
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"UMG",
					
					"Slate",
					"SlateCore",
					"InputCore"
				}
			);
			
		
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"CoreUObject",
					"Engine",
					"Slate",
					"SlateCore",
					"Blutility",
					"UMGEditor",
					"EditorSubsystem",
					"UnrealEd",
					"LevelEditor",
					"ToolMenus",
					"InputCore",
					"ContentBrowser"
				}
			);
		
		
			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
			);
		}
	}
}
