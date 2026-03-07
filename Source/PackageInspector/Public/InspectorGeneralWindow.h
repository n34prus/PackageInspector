#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SInspectorSettingsBlock;
class SInspectorObjectBlock;
class SInspectorDetailsBlock;
class SInspectorPackageBlock;

class SInspectorGeneralWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorGeneralWindow) {}
	SLATE_END_ARGS()

	virtual ~SInspectorGeneralWindow();
	
	void Construct(const FArguments& InArgs);

private:
	
	TSharedPtr<SInspectorSettingsBlock> SettingsBlock;
	TSharedPtr<SInspectorPackageBlock>  PackageBlock;
	TSharedPtr<SInspectorObjectBlock>    ObjectBlock;
	TSharedPtr<SInspectorDetailsBlock>  DetailsBlock;
	
	FDelegateHandle ContentBrowserHandle;

	float UpdateFrequency = 1.0f;	// seconds
	TSharedPtr<FActiveTimerHandle> TimerHandlePtr;
	EActiveTimerReturnType OnTick(double InCurrentTime, float InDeltaTime);
	void UpdateLayout();
	
	void BindToContentBrowser();
	void UnbindFromContentBrowser();
	void OnAssetSelectionChanged(const TArray<FAssetData>& SelectedAssets, bool bIsPrimary);
	void RebuildLayout();

	
};