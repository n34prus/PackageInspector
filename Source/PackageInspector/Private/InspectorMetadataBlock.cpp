#include "InspectorMetadataBlock.h"

TArray<FName> FInspectorMetaDataHelper::GetAvaliableMetaKeys()
{
	TArray<FName> Result;
	if (ExistedCollection.IsEmpty())
	{
		RefreshMetaDataCollection();
	}
	ExistedCollection.GetKeys(Result);
	
	Result.Sort([](const FName& A, const FName& B)
	{
		const bool AIsLastUsed = LastUsedKeys.Contains(A);
		const bool BIsLastUsed = LastUsedKeys.Contains(B);
		if (AIsLastUsed && !BIsLastUsed)
			return true;
		if (!AIsLastUsed && BIsLastUsed)
			return false;
		return A.ToString() < B.ToString();
	});
	return Result;
}

TArray<FString> FInspectorMetaDataHelper::GetAvaliableMetaValues(const FName& Key)
{
	TArray<FString> Result;
	if (ExistedCollection.Contains(Key))
	{
		Result = ExistedCollection[Key].Array();
		Result.Sort();
	}
	return Result;
}

void FInspectorMetaDataHelper::RefreshMetaDataCollection()
{
	ExistedCollection.Reset();
	for (TObjectIterator<UPackage> It; It; ++It)
	{
		UPackage* Package = *It;
		for (const auto& [Obj, MetaDataMap] : GetMetaData(Package))
		{
			for (const auto& [Key, Value] : MetaDataMap)
			{
				auto& Set = ExistedCollection.FindOrAdd(Key);
				if (!Value.IsEmpty()) Set.Add(Value);
			}
		}
	}
}

TMap<UObject*, FInspectorObjectMetaData> FInspectorMetaDataHelper::GetMetaData(const UPackage* Package)
{
	TMap<UObject *, FInspectorObjectMetaData> Result;

	if (!Package) return Result;
	bool bHasMetaData = Package->HasMetaData();
	if (!bHasMetaData) return Result;

	auto ConstPackage = const_cast<UPackage*>(Package);
	if (!ConstPackage)
		return Result;

	for (const auto & Entry : ConstPackage->GetMetaData()->ObjectMetaDataMap)
	{
		if (Entry.Key.IsValid( ))
		{
			Result.Add(Entry.Key.Get( ), FInspectorObjectMetaData{ Entry.Value });
		}
	}

	return Result;
}

TArray<FInspectorObjectMetaData> FInspectorMetaDataHelper::GetMetaDataForUnreachableObjects(const UPackage* Package)
{
	TArray<FInspectorObjectMetaData> Result;

	if (!Package)
		return Result;

	auto ConstPackage = const_cast<UPackage*>(Package);
	if (!ConstPackage)
		return Result;

	for (const auto & Entry : ConstPackage->GetMetaData()->ObjectMetaDataMap)
	{
		if (!Entry.Key.IsValid( ))
		{
			Result.Emplace(FInspectorObjectMetaData{ Entry.Value });
		}
	}

	return Result;
}

void FInspectorMetaDataHelper::SetMetaData(const UObject* Object, const FName Key, const FString Value)
{
	if (!Object || Key.IsNone( )) return;
	LastUsedKeys.Add(Key);
	Object->GetPackage( )->GetMetaData( )->SetValue(Object, Key, *Value);
}

void FInspectorMetaDataHelper::RemoveMetaData(const UObject* Object, const FName Key)
{
	if (!Object || Key.IsNone( )) return;

	Object->GetPackage( )->GetMetaData( )->RemoveValue(Object, Key);
}

FInspectorMetaSelector::FInspectorMetaSelector()
{
	FInspectorMetaDataHelper::RefreshMetaDataCollection();
	for (const auto& Key : FInspectorMetaDataHelper::GetAvaliableMetaKeys())
	{
		SelectorKeys.Add(MakeShared<FName>(Key));
	}	
}

void FInspectorMetaSelector::SetKeyAndGenerateValues(TSharedPtr<FName> NewValue)
{
	SelectedKey = NewValue;
	SelectorValues.Empty();
	if (SelectedKey.IsValid())
	{
		for (const auto& Value : FInspectorMetaDataHelper::GetAvaliableMetaValues(*SelectedKey))
		{
			SelectorValues.Add(MakeShared<FString>(Value));
		}
	}
}

void SInspectorMetaRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
{
	Item = InArgs._Item;
    SMultiColumnTableRow<TSharedPtr<FMetaRow>>::Construct(FSuperRowType::FArguments(), OwnerTable);
}

TSharedRef<SWidget> SInspectorMetaRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	// empty row is used for "add new meta" function 
	const bool bIsEmpty = Item->Key.IsNone( ) && Item->Value.IsEmpty();

	// add new meta
	if (bIsEmpty)
	{
		if (!MetaSelector.IsValid())
		{
			MetaSelector = MakeUnique<FInspectorMetaSelector>();
		}
		
		if (ColumnName == "Key")
		{			
			KeyComboBox = SNew(SComboBox<TSharedPtr<FName>>)
				.OptionsSource(& MetaSelector->SelectorKeys)
				.OnGenerateWidget_Lambda([](TSharedPtr<FName> Item) -> TSharedRef<SWidget>
				{
					return SNew(STextBlock)
						.Text(Item.IsValid() ? FText::FromName(*Item) : FText::GetEmpty());
				})
				.OnSelectionChanged_Lambda([this](TSharedPtr<FName> NewSelected, ESelectInfo::Type)
				{
					MetaSelector->SetKeyAndGenerateValues(NewSelected);
					if (ValueComboBox.IsValid())
					{
						ValueComboBox->RefreshOptions();
					}
				})
				[
					SNew(SEditableTextBox)
					.Text_Lambda([this]() -> FText
					{
						return MetaSelector->SelectedKey.IsValid()
							? FText::FromName(*MetaSelector->SelectedKey)
							: FText::GetEmpty();
					})
					.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type CommitType)
					{
						if (!NewText.IsEmpty())
						{
							TSharedPtr<FName> NewKey = MakeShared<FName>(FName(*NewText.ToString()));
							if (!MetaSelector->SelectorKeys.Contains(NewKey))
							{
								MetaSelector->SelectorKeys.Add(NewKey);
							}
							MetaSelector->SetKeyAndGenerateValues(NewKey);
							if (KeyComboBox.IsValid())
							{
								KeyComboBox->SetSelectedItem(NewKey);
								KeyComboBox->RefreshOptions();
							}
							if (ValueComboBox.IsValid())
							{
								ValueComboBox->RefreshOptions();
							}
						}
					})
				];

			return KeyComboBox.ToSharedRef();
		}

		if (ColumnName == "Value")
		{
			ValueComboBox = SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(& MetaSelector->SelectorValues)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
			{
				return SNew(STextBlock)
					.Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty());
			})
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewSelected, ESelectInfo::Type)
			{
				MetaSelector->SelectedValue = NewSelected;
			})
			[
				SNew(SEditableTextBox)
					.Text_Lambda([this]() -> FText
					{
						return MetaSelector->SelectedValue.IsValid()
							? FText::FromString(*MetaSelector->SelectedValue)
							: FText::GetEmpty();
					})
					.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type CommitType)
					{
						if (!NewText.IsEmpty())
						{
							TSharedPtr<FString> NewValue = MakeShared<FString>(*NewText.ToString());
							MetaSelector->SelectorValues.Add(NewValue);
							if (ValueComboBox.IsValid())
							{
								ValueComboBox->SetSelectedItem(NewValue);
								ValueComboBox->RefreshOptions();
							}

						}
					})
			];
			return ValueComboBox.ToSharedRef();
		}

		if (ColumnName == "Action")
		{
			return SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked_Lambda([this]() -> FReply
				{
					Item->Key = MetaSelector->SelectedKey
						? *MetaSelector->SelectedKey
						: FName("UndefinedKey");
					Item->Value = MetaSelector->SelectedValue
						? *MetaSelector->SelectedValue
						: FString();
					OnAddRequested.ExecuteIfBound(Item);
					return FReply::Handled();
				})
				.Content()
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Plus"))
				];
		}
	}

	// show existed meta
	if (ColumnName == "Key")
	{
		return SNew(STextBlock)
			.Text(FText::FromName(Item->Key));
	}

	if (ColumnName == "Value")
	{
		return SNew(STextBlock)
			.Text(FText::FromString(Item->Value));
	}

	if (ColumnName == "Action")
	{
		return SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.OnClicked_Lambda([this]() -> FReply
			{
				OnDeleteRequested.ExecuteIfBound(Item);
				return FReply::Handled();
			})
			.Content()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.Delete"))
			];
	}

	return SNullWidget::NullWidget;
}

void SInspectorMetadataBlock::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(TableView, SListView<TSharedPtr<FMetaRow>>)
		.ListItemsSource(&MetaRows)
		.OnGenerateRow(this, &SInspectorMetadataBlock::OnGenerateRow)
		.HeaderRow
		(
			SNew(SHeaderRow)
			+ SHeaderRow::Column("Key").DefaultLabel(FText::FromString("Key")).FillWidth(0.4f)
			+ SHeaderRow::Column("Value").DefaultLabel(FText::FromString("Value")).FillWidth(0.4f)
			+ SHeaderRow::Column("Action").DefaultLabel(FText::FromString("Action")).FillWidth(0.2f)
		)
	];
}

void SInspectorMetadataBlock::BindObject(UObject* Object)
{
	TargetObject = Object;
	UpdateLayout();
}

void SInspectorMetadataBlock::UpdateLayout()
{
	MetaRows.Empty();

	if (TargetObject.IsValid( ))
	{
		auto PackageMetaData = FInspectorMetaDataHelper::GetMetaData(TargetObject->GetPackage());
		auto ObjectMetaData = PackageMetaData.Find(TargetObject.Get());
		if (ObjectMetaData)
		{
			for (const auto [Key,Value] : *ObjectMetaData)
			{
				MetaRows.Add(MakeShared<FMetaRow>(FMetaRow{Key, Value}));
			}
		}
	}
	// last empty row is used for "add new meta" function 
	MetaRows.Add(MakeShared<FMetaRow>(FMetaRow{FName(), FString()}));
	TableView->RebuildList();
}

TSharedRef<ITableRow> SInspectorMetadataBlock::OnGenerateRow(TSharedPtr<FMetaRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    const bool bIsLast = (Item == MetaRows.Last());
	const bool bIsEmpty = Item->Key.IsNone( );
	ensureMsgf((!bIsLast || bIsEmpty), TEXT("Last row must be empty, but it is not!"));

	auto Row = SNew(SInspectorMetaRow, OwnerTable).Item(Item);
	Row->OnDeleteRequested.BindSP(SharedThis(this), &SInspectorMetadataBlock::OnDeleteMetaRow);
	Row->OnAddRequested.BindSP(SharedThis(this), &SInspectorMetadataBlock::OnAddMetaRow);
	return Row;
}

void SInspectorMetadataBlock::OnDeleteMetaRow(TSharedPtr<FMetaRow> Row)
{
	FInspectorMetaDataHelper::RemoveMetaData(TargetObject.Get(), Row->Key);
	UpdateLayout();
}

void SInspectorMetadataBlock::OnAddMetaRow(TSharedPtr<FMetaRow> Row)
{
	FInspectorMetaDataHelper::SetMetaData(TargetObject.Get(), Row->Key, Row->Value);
	UpdateLayout();
}
