#include "InspectorGeneralWindow.h"
#include "InspectorSettingsBlock.h"
#include "InspectorObjectBlock.h"
#include "InspectorDetailsBlock.h"
#include "InspectorPackageBlock.h"

#include "ContentBrowserModule.h"
#include "Widgets/Layout/SSplitter.h"

SInspectorGeneralWindow::~SInspectorGeneralWindow()
{
	UnbindFromContentBrowser();
	if (TimerHandlePtr.IsValid())
	{
		UnRegisterActiveTimer(TimerHandlePtr.ToSharedRef());
	}
}

void SInspectorGeneralWindow::Construct(const FArguments& InArgs)
{
	RebuildLayout();
	BindToContentBrowser();
	TimerHandlePtr = RegisterActiveTimer(UpdateFrequency, FWidgetActiveTimerDelegate::CreateSP(this, &SInspectorGeneralWindow::OnTick));
}

void SInspectorGeneralWindow::RebuildLayout()
{
	ChildSlot
	[
		SNew(SSplitter)

		+ SSplitter::Slot()
		.Value(0.1f)
		[
			SAssignNew(SettingsBlock, SInspectorSettingsBlock)
		]

		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SAssignNew(PackageBlock, SInspectorPackageBlock)
		]

		+ SSplitter::Slot()
		.Value(0.4f)
		[
			SAssignNew(ObjectBlock, SInspectorObjectBlock)
		]

		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SAssignNew(DetailsBlock, SInspectorDetailsBlock)
		]
	];

	SettingsBlock->OnSettingsChanged.BindLambda(
[this](bool bAll, bool bTransient, bool bEdit)
		{
			//TreeBlock->SetMode(bAll, bTransient);
			DetailsBlock->SetEditingEnabled(bEdit);
		});

	ObjectBlock->OnObjectSelected.BindLambda(
[this](UObject* Obj)
		{
			if (DetailsBlock)
			{
				DetailsBlock->SetObject(Obj);
			}
		});

	PackageBlock->OnMultipleObjectsSelected.BindLambda(
[this](const TArray<UObject*>& ObjArr)
		{
			if (ObjectBlock)
			{
				ObjectBlock->SetRootObjects(ObjArr);
			}
		});
}

EActiveTimerReturnType SInspectorGeneralWindow::OnTick(double InCurrentTime, float InDeltaTime)
{
	UpdateLayout();
	return EActiveTimerReturnType::Continue;
}

void SInspectorGeneralWindow::UpdateLayout()
{
	// if (PackageBlock) PackageBlock->UpdateLayout(); better to stay on manua update
	if (ObjectBlock) ObjectBlock->UpdateLayout();
	if (DetailsBlock) DetailsBlock->UpdateLayout();
}

void SInspectorGeneralWindow::BindToContentBrowser()
{
	FContentBrowserModule& CBModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	ContentBrowserHandle = CBModule.GetOnAssetSelectionChanged().AddRaw(
		this,
		&SInspectorGeneralWindow::OnAssetSelectionChanged
	);
}

void SInspectorGeneralWindow::UnbindFromContentBrowser()
{
	FContentBrowserModule& CBModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	CBModule.GetOnAssetSelectionChanged().Remove(ContentBrowserHandle);
}

void SInspectorGeneralWindow::OnAssetSelectionChanged(
	const TArray<FAssetData>& SelectedAssets,
	bool bIsPrimary)
{
	if (!SelectedAssets.Num() || !ObjectBlock) return;

	TArray<UObject*> RootObjects;
	
	for (auto AssetData : SelectedAssets)
	{
		UObject* Asset = AssetData.GetAsset();	// immediate loading
		if (!Asset) continue;
		UPackage* Package = Asset->GetPackage();
		if (!Package) continue;
		RootObjects.Add(Package);
	}
	
	ObjectBlock->SetRootObjects(RootObjects);
}
