// Copyright 2017-2018 Csaba Molnar, Daniel Butum

using UnrealBuildTool;

public class DlgSystem : ModuleRules
{
	public DlgSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        // Enable IWYU
        // https://docs.unrealengine.com/latest/INT/Programming/UnrealBuildSystem/IWYUReferenceGuide/index.html
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        //PrivatePCHHeaderFile = "Private/DlgSystemPrivatePCH.h";

        PublicIncludePaths.AddRange(
			new string[] {
				"DlgSystem/Public"
				// ... add public include paths required here ...
			});


		PrivateIncludePaths.AddRange(
			new string[] {
				"DlgSystem/Private",
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
				"Slate"
				// ... add private dependencies that you statically link with here ...
			});

		// We need this dependency when the DlgSystem works in the editor mode/built with editor
		if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				// ... add any modules that your module loads dynamically here ...
			});
	}
}
