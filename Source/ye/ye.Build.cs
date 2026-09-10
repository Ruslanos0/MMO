// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ye : ModuleRules
{
	public ye(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Add module root so subfolders (Auth/, Database/, Inventory/) are found
		PublicIncludePaths.Add(ModuleDirectory);
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			// Инвентарь: UI
			"UMG",
			// Инвентарь: репликация
			"NetCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore",
		});

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// ─── libpq (PostgreSQL C API) ───────────────────────────────────
		string VcpkgRoot = "C:/Users/rusla/Documents/vcpkg/installed/x64-windows";

		// Headers
		PublicIncludePaths.Add(VcpkgRoot + "/include");

		// Libraries - libpq only (no pqxx to avoid allocator conflicts)
		PublicAdditionalLibraries.Add(VcpkgRoot + "/lib/libpq.lib");
		PublicAdditionalLibraries.Add(VcpkgRoot + "/lib/libssl.lib");
		PublicAdditionalLibraries.Add(VcpkgRoot + "/lib/libcrypto.lib");

		// DLLs
		RuntimeDependencies.Add(VcpkgRoot + "/bin/libpq.dll");
		RuntimeDependencies.Add(VcpkgRoot + "/bin/libssl-3-x64.dll");
		RuntimeDependencies.Add(VcpkgRoot + "/bin/libcrypto-3-x64.dll");

		// Required to use third-party C++ headers without UE warnings
		bEnableExceptions = true;
		bUseUnity = false;
	}
}
