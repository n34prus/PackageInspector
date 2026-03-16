#include "InspectorGeneralWindow.h"
#include "InspectorSettingsBlock.h"
#include "InspectorObjectBlock.h"
#include "InspectorDetailsBlock.h"
#include "InspectorPackageBlock.h"
#include "InspectorMetadataBlock.h"
#include "InspectorDirtyBlock.h"

#include "ContentBrowserModule.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

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
	InspectorTabStyle = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckbox");
	InspectorTabStyle.UncheckedImage.TintColor			= FSlateColor(FLinearColor::Transparent);
	InspectorTabStyle.UncheckedHoveredImage.TintColor	= SelectedGrey;
	InspectorTabStyle.UncheckedPressedImage.TintColor	= PressedGrey;
	InspectorTabStyle.CheckedImage.TintColor			= SelectedGrey;
	InspectorTabStyle.CheckedHoveredImage.TintColor		= SelectedGrey;
	InspectorTabStyle.CheckedPressedImage.TintColor		= PressedGrey;
	
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
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(25.f)
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(25.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SCheckBox)
							.Style(&InspectorTabStyle)
							.IsChecked_Lambda([this]()
							{
								return TabSwitcher->GetActiveWidgetIndex() == 1
								? ECheckBoxState::Checked
								: ECheckBoxState::Unchecked;
							})
							.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
							{
								if (State == ECheckBoxState::Checked) TabSwitcher->SetActiveWidgetIndex(1);
							})
							[
								SNew(STextBlock).Text(FText::FromString("Details"))
								.Justification(ETextJustify::Center)
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SCheckBox)
							.Style(&InspectorTabStyle)
							.IsChecked_Lambda([this]()
							{
								return TabSwitcher->GetActiveWidgetIndex() == 2
								? ECheckBoxState::Checked
								: ECheckBoxState::Unchecked;
							})
							.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
							{
								if (State == ECheckBoxState::Checked) TabSwitcher->SetActiveWidgetIndex(2);
							})
							[
								SNew(STextBlock).Text(FText::FromString("Metadata"))
								.Justification(ETextJustify::Center)
							]
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SCheckBox)
							.Style(&InspectorTabStyle)
							.IsChecked_Lambda([this]()
							{
								return TabSwitcher->GetActiveWidgetIndex() == 0
								? ECheckBoxState::Checked
								: ECheckBoxState::Unchecked;
							})
							.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
							{
								if (State == ECheckBoxState::Checked) TabSwitcher->SetActiveWidgetIndex(0);
							})
							[
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								[
									SNew(SImage)
									.Image(FCoreStyle::Get().GetBrush("Icons.Settings"))
								]
							]
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Bottom)
					[
						SNew(SBox)
						.HeightOverride(3.f)
						[
							SNew(SColorBlock)
							.Color(SelectedGrey)
						]
					]
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				SAssignNew(TabSwitcher, SWidgetSwitcher)
				
				+ SWidgetSwitcher::Slot()
				[
					SAssignNew(SettingsBlock, SInspectorSettingsBlock)
				]
				
				+ SWidgetSwitcher::Slot()
				[
					SAssignNew(DetailsBlock, SInspectorDetailsBlock)
				]

				+ SWidgetSwitcher::Slot()
				[
					SAssignNew(MetadataBlock, SInspectorMetadataBlock)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(50.f)
				.VAlign(VAlign_Center)
				[
					SAssignNew(DirtyBlock, SInspectorDirtyStatusBlock)
				]
			]
			
		]
	];

	SettingsBlock->OnSettingsChanged.BindLambda(
[this](bool bAll, bool bTransient, bool bEdit)
		{
			if (DetailsBlock) DetailsBlock->SetEditingEnabled(bEdit);
		});

	ObjectBlock->OnObjectSelected.BindLambda(
[this](UObject* Obj)
		{
			if (DetailsBlock) DetailsBlock->BindObject(Obj);
			if (MetadataBlock) MetadataBlock->BindObject(Obj);
			if (DirtyBlock) DirtyBlock->BindObject(Obj);
		});

	PackageBlock->OnMultipleObjectsSelected.BindLambda(
[this](const TArray<UObject*>& ObjArr)
		{
			if (ObjectBlock) ObjectBlock->SetRootObjects(ObjArr);
		});

	TabSwitcher->SetActiveWidgetIndex(1);
}

EActiveTimerReturnType SInspectorGeneralWindow::OnTick(double InCurrentTime, float InDeltaTime)
{
	UpdateLayout();
	return EActiveTimerReturnType::Continue;
}

void SInspectorGeneralWindow::UpdateLayout()
{
	// broadcast update to any external logic out of basic childs
	OnUpdateLayout.Broadcast();
	// if (PackageBlock) PackageBlock->UpdateLayout(); // better to stay on manual update
	if (ObjectBlock) ObjectBlock->UpdateLayout();
	if (DetailsBlock) DetailsBlock->UpdateLayout();
	if (DirtyBlock) DirtyBlock->UpdateLayout();
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
	auto CBModule =
	FModuleManager::GetModulePtr<FContentBrowserModule>("ContentBrowser");
	if (!CBModule) return;
	CBModule->GetOnAssetSelectionChanged().Remove(ContentBrowserHandle);
}

void SInspectorGeneralWindow::OnAssetSelectionChanged(
	const TArray<FAssetData>& SelectedAssets,
	bool bIsPrimary)
{
	if (!SelectedAssets.Num() || !ObjectBlock) return;

	TArray<UObject*> RootObjects;
	
	for (auto AssetData : SelectedAssets)
		{
		UObject* Asset = AssetData.GetAsset(); // immediate loading
		if (!Asset) continue;
		UPackage* Package = Asset->GetPackage();
		if (!Package) continue;
		RootObjects.Add(Package);
	}
	
	ObjectBlock->SetRootObjects(RootObjects);
}
