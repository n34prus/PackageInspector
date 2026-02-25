#include "InspectorTreeView.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

#include "Engine/Blueprint.h"
#include "UObject/UObjectIterator.h"

void SInspectorTreeView::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(TreeViewWidget, STreeView<FNodePtr>)
		.TreeItemsSource(&RootItems)
		.OnGenerateRow(this, &SInspectorTreeView::OnGenerateRow)
		.OnGetChildren(this, &SInspectorTreeView::OnGetChildren)
		.OnSelectionChanged(this, &SInspectorTreeView::OnSelectionChanged)
	];

	BindToContentBrowser();
}

SInspectorTreeView::~SInspectorTreeView()
{
}

void SInspectorTreeView::BindToContentBrowser()
{
	FContentBrowserModule& CBModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	CBModule.GetOnAssetSelectionChanged().AddRaw(
		this,
		&SInspectorTreeView::OnAssetSelectionChanged
	);
}

void SInspectorTreeView::OnAssetSelectionChanged(
	const TArray<FAssetData>& SelectedAssets,
	bool bIsPrimary)
{
	if (SelectedAssets.Num() == 0)
		return;

	RootItems.Empty();
	ChildrenMap.Empty();

	BuildTreeFromAsset(SelectedAssets[0]);

	TreeViewWidget->RequestTreeRefresh();
}

void SInspectorTreeView::BuildTreeFromAsset(const FAssetData& AssetData)
{
	UObject* Asset = AssetData.GetAsset();
	if (!Asset)
		return;

	UPackage* Package = Asset->GetPackage();
	if (!Package)
		return;
	
	auto RootNode = MakeShared<FInspectorNode>(Package);
	RootItems.Add(RootNode);

	ExtractPackageObjects(Package, RootNode);
}

void SInspectorTreeView::ExtractPackageObjects(UPackage* Package, FNodePtr Parent)
{
	TArray<UObject*> Objects;
	GetObjectsWithOuter(Package, Objects, true);

	for (UObject* Obj : Objects)
	{
		auto Node = MakeShared<FInspectorNode>(Obj);

		ChildrenMap.FindOrAdd(Parent).Add(Node);

		if (UClass* Class = Cast<UClass>(Obj))
		{
			if (UObject* CDO = Class->GetDefaultObject())
			{
				ChildrenMap.FindOrAdd(Node).Add(
					MakeShared<FInspectorNode>(CDO));
			}
		}
	}
}

TSharedRef<ITableRow> SInspectorTreeView::OnGenerateRow(
	FNodePtr Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	UObject* Obj = Item.Get()->Get();

	FString Label = Obj
		? FString::Printf(TEXT("%s (%s)"),
			*Obj->GetName(),
			*Obj->GetClass()->GetName())
		: TEXT("Invalid");

	return SNew(STableRow<FNodePtr>, OwnerTable)
	[
		SNew(STextBlock).Text(FText::FromString(Label))
	];
}

void SInspectorTreeView::OnGetChildren(
	FNodePtr Item,
	TArray<FNodePtr>& OutChildren) const
{
	if (ChildrenMap.Contains(Item))
	{
		OutChildren = ChildrenMap[Item].Array();
	}
}

void SInspectorTreeView::OnSelectionChanged(
	FNodePtr Item,
	ESelectInfo::Type SelectInfo)
{
	UObject* Selected = Item.IsValid()
			? Item.Get()->Get()
			: nullptr;

	OnObjectSelected.ExecuteIfBound(Selected);
}