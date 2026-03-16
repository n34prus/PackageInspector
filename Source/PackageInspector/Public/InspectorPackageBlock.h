#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UObject/Package.h"
#include "PackageInspector.h"

struct FPackageTreeNode
{
	FString Name;
	FString FullPath;
	TWeakObjectPtr<UPackage> Package; // null for folder
	TArray<TSharedPtr<FPackageTreeNode>> Children;

	// filters
	bool bHighlighted = false;
	bool bHasPathMatch = false;  
	bool bHasNameMatch = false;
	bool bHasLeafMatch = false;

	bool IsFolder() const { return !Package.IsValid(); }
};

using FInspectPackagePtr = TSharedPtr<FPackageTreeNode>;

class SInspectorPackageRow : public SMultiColumnTableRow<FInspectPackagePtr>
{
public:
    SLATE_BEGIN_ARGS(SInspectorPackageRow) {}
	SLATE_ARGUMENT(FInspectPackagePtr, Item)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable);
    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	
    FInspectPackagePtr Item;
};

class SInspectorPackageBlock : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorPackageBlock) {}
	SLATE_END_ARGS()

	FOnMultipleObjectsSelected OnMultipleObjectsSelected;
	
	void Construct(const FArguments& InArgs);
	void UpdatePackages();
	void UpdateLayout();


private:
	
	void OnSelectionChanged(FInspectPackagePtr Item,	ESelectInfo::Type SelectInfo);
	void OnGetChildren(FInspectPackagePtr InItem,	TArray<FInspectPackagePtr>& OutChildren);
	TSharedRef<ITableRow> OnGenerateRow(FInspectPackagePtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void CollectPackagesRecursive(const FInspectPackagePtr& Node,	TArray<UObject*>& Out);
	void SortTree(TArray<FInspectPackagePtr>& Nodes);

	void OnSearchChanged(const FText& InText);
	void ApplyFilter();
	bool FilterNode(const FInspectPackagePtr& Node,	FInspectPackagePtr& OutNode);

	FReply OnRefreshPressed();
	
	TSharedPtr<STreeView<FInspectPackagePtr>> TreeView;
	TArray<FInspectPackagePtr> RootNodes;
	TArray<FInspectPackagePtr> AllRootNodes;

	TSharedPtr<SSearchBox> SearchBox;
	FString SearchString;
};