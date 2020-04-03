// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DlgSystemEditor : ModuleRules
{
	public DlgSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable IWYU
		// https://docs.unrealengine.com/latest/INT/Programming/UnrealBuildSystem/IWYUReferenceGuide/index.html
		// https://docs.unrealengine.com/latest/INT/Programming/UnrealBuildSystem/Configuration/
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		//MinFilesUsingPrecompiledHeaderOverride = 1;
		// bFasterWithoutUnity = true;
		// bUseUnity = false;
		//PrivatePCHHeaderFile = "Private/DlgSystemEditorPrivatePCH.h";

		//if ((Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
		//	&& WindowsPlatform.Compiler >= WindowsCompiler.VisualStudio2015)
		//{
			//Definitions.Add("/Wall");
		//}

		// Docs see https://docs.unrealengine.com/latest/INT/Programming/UnrealBuildSystem/ModuleFiles/

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public")
				// ... add public include paths required here ...
			});


		string PrivateDirectory = Path.Combine(ModuleDirectory, "Private");
		PrivateIncludePaths.AddRange(
			new string[] {
				PrivateDirectory,
				Path.Combine(PrivateDirectory, "Commandlets")
				// ... add other private include paths required here ...
			});


		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"DlgSystem",
				// ... add other public dependencies that you statically link with here ...
			});


		PrivateDependencyModuleNames.AddRange(
			new string[] {
				// Core modules
				"Engine",
				"CoreUObject",
				"Projects", // IPluginManager
				"UnrealEd", // for FAssetEditorManager
				"RenderCore",
				"InputCore",
				"SlateCore",
				"Slate",
				"EditorStyle",
				"MessageLog",
				"EditorWidgets",

				// Accessing the menu
				"WorkspaceMenuStructure",

				// Details/Property editor
				"DetailCustomizations",
				"PropertyEditor",

				// Used for the Blueprint Nodes
				"BlueprintGraph",
				"Kismet",
				"KismetCompiler",
				"KismetWidgets",

				// graph stuff
				"GraphEditor",
				"ContentBrowser",

				// e.g. FPlatformApplicationMisc::ClipboardCopy
				"ApplicationCore",
			});

#if UE_4_24_OR_LATER
		PrivateDependencyModuleNames.Add("ToolMenus");
#endif

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetRegistry",
				"AssetTools",
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				// ... add any modules that your module loads dynamically here ...
			});
	}
}
