#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "PackageInspector.h"

struct FAssetData;

using FInspectObjectPtr = TWeakObjectPtr<UObject>;
	
class SInspectorObjectRow : public SMultiColumnTableRow<FInspectObjectPtr>
{
public:
	SLATE_BEGIN_ARGS(SInspectorObjectRow) {}
	SLATE_ARGUMENT(FInspectObjectPtr, Item)
	SLATE_END_ARGS()

void Construct(
	const FArguments& InArgs,
	const TSharedRef<STableViewBase>& OwnerTable);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(
	const FName& ColumnName) override;

private:
	FInspectObjectPtr Item;
};

class SInspectorObjectBlock : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SInspectorObjectBlock) {}
	SLATE_END_ARGS()

	FOnObjectSelected OnObjectSelected;
	
	void Construct(const FArguments& InArgs);
	virtual ~SInspectorObjectBlock();

	void AddRootObjects(const TArray<UObject*>& RootObjects, bool ClearRoot = false);
	inline void AddRootObject(UObject* RootObject, bool ClearRoot = false) { AddRootObjects({RootObject}, ClearRoot); }
	inline void SetRootObjects(const TArray<UObject*>& RootObjects) { AddRootObjects(RootObjects, true); }
	inline void SetRootObject(UObject* RootObject) { AddRootObjects({RootObject}, true); }
	
	void RemoveRootObjects(const TArray<UObject*>& RootObject);
	inline void RemoveRootObject(UObject* RootObject) { RemoveRootObjects({RootObject}); }

	void UpdateLayout();

private:
	
	TSharedPtr<STreeView<FInspectObjectPtr>> TreeView;
	TSharedPtr<STextBlock> HeadHintTextLeft;
	TSharedPtr<STextBlock> HeadHintTextRight;

	TArray<FInspectObjectPtr> RootItems;

	TMap<FInspectObjectPtr, TSet<FInspectObjectPtr>> ChildrenMap;
	
	void ExtractPackageObjects(FInspectObjectPtr RootNode, uint8_t depth = 0);
	
	TSharedRef<ITableRow> OnGenerateRow(FInspectObjectPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(FInspectObjectPtr Item, TArray<FInspectObjectPtr>& OutChildren);
	void OnSelectionChanged(FInspectObjectPtr Item, ESelectInfo::Type SelectInfo);
	void OnItemExpansionChanged(FInspectObjectPtr Item, bool bExpanded);
	void UpdateHint();
	
	// context menu //
	TSharedPtr<SWidget> OnContextMenuOpening();
	void CmOnRenameCommitted(const FText& Text, ETextCommit::Type Arg, UObject* Object);
	void RenameSelectedObject();
	void CmOnRemoveFromPackage();
	void CmOnDestroyObject();
	void CmCopyPathToClipboard();
	void CmCopyAddressToClipboard();
	void CmCreateSubObject();
	void OnNewSubObjectNameCommitted(const FText& Text, ETextCommit::Type Arg, UClass* Class);

};


