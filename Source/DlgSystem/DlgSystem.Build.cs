// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DlgSystem : ModuleRules
{
	public DlgSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable IWYU
		// https://docs.unrealengine.com/latest/INT/Programming/UnrealBuildSystem/IWYUReferenceGuide/index.html
		// https://docs.unrealengine.com/latest/INT/Programming/UnrealBuildSystem/Configuration/
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		//MinFilesUsingPrecompiledHeaderOverride = 1;
		// bFasterWithoutUnity = true;
		// bUseUnity = false;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public")
				// ... add public include paths required here ...
			});


		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private")
				// ... add other private include paths required here ...
			});


		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"Json",
				"JsonUtilities"
				// ... add other public dependencies that you statically link with here ...
			});


		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"CoreUObject",
				"Engine",
				"Projects", // IPluginManager

				// UI
				"SlateCore",
				"Slate",
				"InputCore"
				// ... add private dependencies that you statically link with here ...
			});

		// Add MessageLog support
		if (Target.bBuildDeveloperTools)
		{
			PrivateDependencyModuleNames.Add("MessageLog");
		}

		// We need this dependency when the DlgSystem works in the editor mode/built with editor
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("EditorWidgets");
			PrivateDependencyModuleNames.Add("UnrealEd");

			// Accessing the menu
			PrivateDependencyModuleNames.Add("WorkspaceMenuStructure");
		}

		// Add GameplayDebugger functionality if not 'Shipping' or 'Test' Target.
		if (Target.bBuildDeveloperTools ||
			(Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
		{
			PrivateDependencyModuleNames.Add("GameplayDebugger");
			PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1");
		}
		else
		{
			PublicDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				// ... add any modules that your module loads dynamically here ...
			});
	}
}
