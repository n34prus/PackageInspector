// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "InspectorGeneralWindow.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPackageInspector, Log, All);

DECLARE_DELEGATE_OneParam(
	FOnObjectSelected,
	UObject*
);

DECLARE_DELEGATE_OneParam(
	FOnMultipleObjectsSelected,
	const TArray<UObject*>&
);

class FPackageInspectorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	TSharedPtr<SInspectorGeneralWindow> GetWindow() const { return GeneralWindow; }
	inline static FPackageInspectorModule& Get();

private:
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<SInspectorGeneralWindow> GeneralWindow;
};
