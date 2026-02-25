// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetInspector.h"

#include "ToolMenus.h"
#include "InspectorGeneralWindow.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FAssetInspectorModule"

static const FName AssetInspectorTabName("AssetInspector");

void FAssetInspectorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		AssetInspectorTabName,
		FOnSpawnTab::CreateRaw(this, &FAssetInspectorModule::OnSpawnPluginTab)
	)
	.SetDisplayName(FText::FromString("Asset Inspector"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
	FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenu* Menu =
			UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");

		FToolMenuSection& Section =
			Menu->AddSection("AssetInspector");

		Section.AddMenuEntry(
			"OpenAssetInspector",
			FText::FromString("Asset Inspector"),
			FText::FromString("Open Asset Inspector"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(
					FName("AssetInspector"));
			}))
		);
	})
);
}

void FAssetInspectorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AssetInspectorTabName);
}

TSharedRef<SDockTab> FAssetInspectorModule::OnSpawnPluginTab(
	const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SInspectorGeneralWindow)
		];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetInspectorModule, AssetInspector)