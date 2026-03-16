// Copyright Epic Games, Inc. All Rights Reserved.

#include "PackageInspector.h"

#include "ToolMenus.h"
#include "InspectorGeneralWindow.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FPackageInspectorModule"

DEFINE_LOG_CATEGORY(LogPackageInspector);

static const FName PackageInspectorTabName("PackageInspector");

void FPackageInspectorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		PackageInspectorTabName,
		FOnSpawnTab::CreateRaw(this, &FPackageInspectorModule::OnSpawnPluginTab)
	)
	.SetDisplayName(FText::FromString("Package Inspector"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
	FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenu* Menu =
			UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");

		FToolMenuSection& Section =	Menu->FindOrAddSection(
			"Plugins",
			FText::FromString("Plugins"),
			FToolMenuInsert("Tools", EToolMenuInsertType::After));

		Section.AddMenuEntry(
			"PackageInspector",
			FText::FromString("Package Inspector"),
			FText::FromString("Open Package Inspector"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Tool"),	// doesn't work
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(
					FName("PackageInspector"));
			}))
		);
	}));
}

void FPackageInspectorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PackageInspectorTabName);
}

FPackageInspectorModule& FPackageInspectorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FPackageInspectorModule>("FPackageInspectorModule");
}

TSharedRef<SDockTab> FPackageInspectorModule::OnSpawnPluginTab(
	const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SAssignNew(GeneralWindow, SInspectorGeneralWindow)
		];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPackageInspectorModule, PackageInspector)