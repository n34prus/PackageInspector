#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_MULTICAST_DELEGATE(FOnUpdateLayout);

class SInspectorSettingsBlock;
class SInspectorObjectBlock;
class SInspectorDetailsBlock;
class SInspectorPackageBlock;
class SInspectorMetadataBlock;
class SInspectorDirtyStatusBlock;

class SInspectorGeneralWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorGeneralWindow) {}
	SLATE_END_ARGS()

	virtual ~SInspectorGeneralWindow();
	
	void Construct(const FArguments& InArgs);

	FOnUpdateLayout OnUpdateLayout;

private:
	
	TSharedPtr<SInspectorSettingsBlock> SettingsBlock;
	TSharedPtr<SInspectorPackageBlock>  PackageBlock;
	TSharedPtr<SInspectorObjectBlock>    ObjectBlock;
	TSharedPtr<SInspectorDetailsBlock>  DetailsBlock;
	TSharedPtr<SInspectorMetadataBlock>  MetadataBlock;
	TSharedPtr<SInspectorDirtyStatusBlock> DirtyBlock;
	TSharedPtr<SWidgetSwitcher> TabSwitcher;
	
	FDelegateHandle ContentBrowserHandle;

	float UpdateFrequency = 1.0f;	// seconds
	TSharedPtr<FActiveTimerHandle> TimerHandlePtr;
	FCheckBoxStyle InspectorTabStyle;
	static constexpr FLinearColor SelectedGrey {0.03f, 0.03f, 0.03f, 1.0f};
	static constexpr FLinearColor PressedGrey {0.02f, 0.02f, 0.02f, 1.0f};
	
	EActiveTimerReturnType OnTick(double InCurrentTime, float InDeltaTime);
	void UpdateLayout();
	
	void BindToContentBrowser();
	void UnbindFromContentBrowser();
	void OnAssetSelectionChanged(const TArray<FAssetData>& SelectedAssets, bool bIsPrimary);
	void RebuildLayout();

	
};