#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

struct FAssetData;

DECLARE_DELEGATE_OneParam(
	FOnObjectSelected,
	UObject*
);

struct FInspectorNode
{
	FInspectorNode(UObject* InObject) : Object(InObject) {}
	
	TWeakObjectPtr<UObject> Object;
		
	FORCEINLINE UObject* Get() const
	{
		return Object.Get();
	}

	FORCEINLINE bool IsValid() const
	{
		return Object.IsValid();
	}
		
	bool operator==(const FInspectorNode& Other) const
	{
		return Object.Get() == Other.Object.Get();
	}
};

FORCEINLINE uint32 GetTypeHash(const FInspectorNode& Node)
{
	return GetTypeHash(Node.Object);
}

class SInspectorTreeView : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SInspectorTreeView) {}
	SLATE_END_ARGS()

	FOnObjectSelected OnObjectSelected;
	
	void Construct(const FArguments& InArgs);
	virtual ~SInspectorTreeView();

private:

	using FNodePtr = TSharedPtr<FInspectorNode>;
	
	TSharedPtr<STreeView<FNodePtr>> TreeViewWidget;

	TArray<FNodePtr> RootItems;

	TMap<FNodePtr, TSet<FNodePtr>> ChildrenMap;
	
	void BindToContentBrowser();
	void OnAssetSelectionChanged(const TArray<FAssetData>& SelectedAssets, bool bIsPrimary);
	
	void BuildTreeFromAsset(const FAssetData& AssetData);
	void ExtractPackageObjects(UPackage* Package, FNodePtr Parent = nullptr);
	
	TSharedRef<ITableRow> OnGenerateRow(FNodePtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(FNodePtr Item, TArray<FNodePtr>& OutChildren) const;
	void OnSelectionChanged(FNodePtr Item, ESelectInfo::Type SelectInfo);
};