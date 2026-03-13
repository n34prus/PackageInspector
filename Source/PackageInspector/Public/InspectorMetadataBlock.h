#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
//#include "Widgets/Views/STreeView.h"
#include "PackageInspector.h"

using FInspectorObjectMetaData = TMap<FName, FString>;
struct FMetaRow
{
	FName Key;
	FString Value;
};

DECLARE_DELEGATE_OneParam(
	FOnInspectorMetaActionRequest,
	TSharedPtr<FMetaRow>
);

class InspectorMetaDataHelper
{
public:
	static TArray<FName> GetAvalibleMetaKeys();
	static TArray<FString> GetAvalibleMetaValues(const FName& Key);
	static void RefreshMetaDataCollection();
	
	static TMap<UObject*, FInspectorObjectMetaData> GetMetaData(const UPackage * Package);
	static TArray<FInspectorObjectMetaData> GetMetaDataForUnreachableObjects(const UPackage * Package);
	static void SetMetaData(const UObject * Object, const FName Key, const FString Value);
	static void RemoveMetaData(const UObject * Object, const FName Key);
private:
	inline static TMap<FName, TSet<FString>> ExistedCollection {};
	inline static TSet<FName> LastUsedKeys {};
};

struct FInspectorMetaSelector
{
	FInspectorMetaSelector();
	void SetKeyAndGenerateValues(TSharedPtr<FName> NewValue);
	TArray<TSharedPtr<FName>> SelectorKeys;
	TSharedPtr<FName> SelectedKey;
	TArray<TSharedPtr<FString>> SelectorValues;
	TSharedPtr<FString> SelectedValue;
};

class SInspectorMetaRow : public SMultiColumnTableRow<TSharedPtr<FMetaRow>>
{
public:
	SLATE_BEGIN_ARGS(SInspectorMetaRow) {}
	SLATE_ARGUMENT(TSharedPtr<FMetaRow>, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable);
	FOnInspectorMetaActionRequest OnDeleteRequested;
	FOnInspectorMetaActionRequest OnAddRequested;
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
private:
	TSharedPtr<FMetaRow> Item;
	TUniquePtr<FInspectorMetaSelector> MetaSelector;
	TSharedPtr<SComboBox<TSharedPtr<FName>>> KeyComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ValueComboBox;
};

class SInspectorMetadataBlock : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorMetadataBlock) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetTargetObject(UObject* Object);
	void UpdateLayout();

private:	
	TArray<TSharedPtr<FMetaRow>> MetaRows;
	TWeakObjectPtr<UObject> TargetObject;
	TSharedPtr<SListView<TSharedPtr<FMetaRow>>> TableView;
	TArray<TSharedPtr<FString>> AvailableKeys {MakeShared<FString>("KeyA"), MakeShared<FString>("KeyB")};
	TSharedPtr<FString> SelectedKey;

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FMetaRow> Item, const TSharedRef<STableViewBase>& OwnerTable);

	void OnDeleteMetaRow(TSharedPtr<FMetaRow> Row);
	void OnAddMetaRow(TSharedPtr<FMetaRow> Row);
};