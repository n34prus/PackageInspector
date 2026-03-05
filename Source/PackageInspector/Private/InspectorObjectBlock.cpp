#include "InspectorObjectBlock.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "UObject/UObjectIterator.h"
#include "HAL/PlatformApplicationMisc.h"

void SInspectorObjectRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
{
	Item = InArgs._Item;

	SMultiColumnTableRow<FInspectObjectPtr>::Construct(
		SMultiColumnTableRow<FInspectObjectPtr>::FArguments(),
		OwnerTable);
}

TSharedRef<SWidget> SInspectorObjectRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	UObject* Obj = Item.Get();
	if (!Obj)
	return SNew(STextBlock).Text(FText::FromString("Invalid"));
			
	if (ColumnName == "Class")
	{
		return SNew(SHorizontalBox)
					
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SExpanderArrow, SharedThis(this))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Obj->GetClass()->GetName()))
			];
	}
			
	if (ColumnName == "Name")
	return SNew(STextBlock)
			.Text(FText::FromString(
				Obj ? Obj->GetName() : TEXT("-")));

	if (ColumnName == "Path")
	return SNew(STextBlock)
			.Text(FText::FromString(
				Obj ? Obj->GetPathName() : TEXT("-")));

	return SNew(STextBlock).Text(FText::FromString("Invalid"));
}

void SInspectorObjectBlock::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(50.f)
			.VAlign(VAlign_Center)
			[
				SAssignNew(HeadHintText, STextBlock)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SAssignNew(TreeView, STreeView<FInspectObjectPtr>)
			.TreeItemsSource(&RootItems)
			.OnGenerateRow(this, &SInspectorObjectBlock::OnGenerateRow)
			.OnGetChildren(this, &SInspectorObjectBlock::OnGetChildren)
			.OnSelectionChanged(this, &SInspectorObjectBlock::OnSelectionChanged)
			.OnContextMenuOpening(this, &SInspectorObjectBlock::OnContextMenuOpening)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Class").DefaultLabel(FText::FromString("Class"))
				+ SHeaderRow::Column("Name").DefaultLabel(FText::FromString("Name"))
				+ SHeaderRow::Column("Path").DefaultLabel(FText::FromString("Path"))
			)
		]
	];

	UpdateHint();
}

SInspectorObjectBlock::~SInspectorObjectBlock()
{
}

void SInspectorObjectBlock::AddRootObjects(const TArray<UObject*>& RootObjects, bool ClearRoot)
{
	if (ClearRoot)
	{
		RootItems.Empty();
		ChildrenMap.Empty();
	}
	for (UObject* Obj : RootObjects)
	{
		if (RootItems.Contains(Obj)) continue;
		FInspectObjectPtr RootNode (Obj);
		RootItems.Add(RootNode);
		ExtractPackageObjects(Obj, RootNode, 1);
	}
	TreeView->RequestTreeRefresh();
	UpdateHint();
}

void SInspectorObjectBlock::RemoveRootObjects(const TArray<UObject*>& RootObject)
{
	for (UObject* Obj : RootObject)
		if (RootItems.Contains(Obj)) RootItems.Remove(Obj);
	TreeView->RequestTreeRefresh();
	UpdateHint();
}

void SInspectorObjectBlock::ExtractPackageObjects(UObject* RootObject, FInspectObjectPtr RootNode, uint8_t depth)
{
	if (!RootObject) return;
	TArray<UObject*> Objects;
	GetObjectsWithOuter(RootObject, Objects, false);

	for (UObject* Obj : Objects)
	{
		FInspectObjectPtr Node (Obj);
		
		ChildrenMap.FindOrAdd(RootNode).Add(Node);

		if (depth) ExtractPackageObjects(Obj, Node, depth - 1);
	}
}

TSharedRef<ITableRow> SInspectorObjectBlock::OnGenerateRow(
	FInspectObjectPtr Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SInspectorObjectRow, OwnerTable).Item(Item);
}

void SInspectorObjectBlock::OnGetChildren(
	FInspectObjectPtr Item,
	TArray<FInspectObjectPtr>& OutChildren)
{
	if (ChildrenMap.Contains(Item))
	{
		UObject* Obj = Item.Get();
		ExtractPackageObjects(Obj, Item, 1);
		OutChildren = ChildrenMap[Item].Array();
	}
}

void SInspectorObjectBlock::OnSelectionChanged(
	FInspectObjectPtr Item,
	ESelectInfo::Type SelectInfo)
{
	UObject* Selected = Item.IsValid()
			? Item.Get()
			: nullptr;

	UpdateHint();
	OnObjectSelected.ExecuteIfBound(Selected);
}

TSharedPtr<SWidget> SInspectorObjectBlock::OnContextMenuOpening()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Copy To Clipboard"),
		FText::FromString("Copy selected package paths"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(
				this,
				&SInspectorObjectBlock::CopySelectionToClipboard)
		)
	);

	return MenuBuilder.MakeWidget();
}

void SInspectorObjectBlock::CopySelectionToClipboard()
{
	TArray<FInspectObjectPtr> Selected;
	TreeView->GetSelectedItems(Selected);

	FString Result;

	// probably there is only one selected item, but still
	for (FInspectObjectPtr Item : Selected)
	{
		UObject* Obj = Item.Get();
		if (Obj)
		{
			Result += Obj->GetPathName();
			Result += "\n";
		}
	}

	FPlatformApplicationMisc::ClipboardCopy(*Result);
}

void SInspectorObjectBlock::UpdateHint()
{
	FInspectObjectPtr Selection;

	const auto& SelectedItems = TreeView->GetSelectedItems();
	if (!SelectedItems.IsEmpty()) Selection = SelectedItems[0];
	
	if (UObject* Object = Selection.IsValid() ? Selection.Get()	: nullptr)
	{
		if (const auto Package = Object->GetPackage())
		{
			FString Out;
			uint32 Flags;
			auto AddFlag = [&](uint32 Flag, const TCHAR* Name)
			{
				if (Flags & Flag)
				{
					Out += Name;
					Out += TEXT(" ");
				}
			};
			if (Package == Object)	// selected item is package
			{
				Flags = Package->GetPackageFlags();
				AddFlag(PKG_TransientFlags, TEXT("PKG_Transient"));
				AddFlag(PKG_NewlyCreated, TEXT("PKG_NewlyCreated"));
				AddFlag(PKG_ClientOptional, TEXT("PKG_ClientOptional"));
				AddFlag(PKG_ServerSideOnly, TEXT("PKG_ServerSideOnly"));
				AddFlag(PKG_CompiledIn, TEXT("PKG_CompiledIn"));
				AddFlag(PKG_ForDiffing, TEXT("PKG_ForDiffing"));
				AddFlag(PKG_EditorOnly, TEXT("PKG_EditorOnly"));
				AddFlag(PKG_Developer, TEXT("PKG_Developer"));
				AddFlag(PKG_UncookedOnly, TEXT("PKG_UncookedOnly"));
				AddFlag(PKG_Cooked, TEXT("PKG_Cooked"));
				AddFlag(PKG_ContainsNoAsset, TEXT("PKG_ContainsNoAsset"));
				AddFlag(PKG_NotExternallyReferenceable, TEXT("PKG_NotExternallyReferenceable"));
				AddFlag(PKG_UnversionedProperties, TEXT("PKG_UnversionedProperties"));
				AddFlag(PKG_ContainsMapData, TEXT("PKG_ContainsMapData"));
				AddFlag(PKG_IsSaving, TEXT("PKG_IsSaving"));
				AddFlag(PKG_Compiling, TEXT("PKG_Compiling"));
				AddFlag(PKG_ContainsMap, TEXT("PKG_ContainsMap"));
				AddFlag(PKG_RequiresLocalizationGather, TEXT("PKG_RequiresLocalizationGather"));
				AddFlag(PKG_PlayInEditor, TEXT("PKG_PlayInEditor"));
				AddFlag(PKG_ContainsScript, TEXT("PKG_ContainsScript"));
				AddFlag(PKG_DisallowExport, TEXT("PKG_DisallowExport"));
				AddFlag(PKG_CookGenerated, TEXT("PKG_CookGenerated"));
				AddFlag(PKG_DynamicImports, TEXT("PKG_DynamicImports"));
				AddFlag(PKG_RuntimeGenerated, TEXT("PKG_RuntimeGenerated"));
				AddFlag(PKG_ReloadingForCooker, TEXT("PKG_ReloadingForCooker"));
				AddFlag(PKG_FilterEditorOnly, TEXT("PKG_FilterEditorOnly")); 
			}
			if (true) // objects: object flags only; packages: object & package flags both
			{
				Flags = Object->GetFlags();
				AddFlag(RF_Public, TEXT("RF_Public"));
				AddFlag(RF_Standalone, TEXT("RF_Standalone"));
				AddFlag(RF_MarkAsNative, TEXT("RF_MarkAsNative"));
				AddFlag(RF_Transactional, TEXT("RF_Transactional"));
				AddFlag(RF_ClassDefaultObject, TEXT("RF_ClassDefaultObject"));
				AddFlag(RF_ArchetypeObject, TEXT("RF_ArchetypeObject"));
				AddFlag(RF_Transient, TEXT("RF_Transient"));
				AddFlag(RF_MarkAsRootSet, TEXT("RF_MarkAsRootSet"));
				AddFlag(RF_TagGarbageTemp, TEXT("RF_TagGarbageTemp"));
				AddFlag(RF_NeedInitialization, TEXT("RF_NeedInitialization"));
				AddFlag(RF_NeedLoad, TEXT("RF_NeedLoad"));
				AddFlag(RF_KeepForCooker, TEXT("RF_KeepForCooker"));
				AddFlag(RF_NeedPostLoad, TEXT("RF_NeedPostLoad"));
				AddFlag(RF_NeedPostLoadSubobjects, TEXT("RF_NeedPostLoadSubobjects"));
				AddFlag(RF_NewerVersionExists, TEXT("RF_NewerVersionExists"));
				AddFlag(RF_BeginDestroyed, TEXT("RF_BeginDestroyed"));
				AddFlag(RF_FinishDestroyed, TEXT("RF_FinishDestroyed"));
				AddFlag(RF_BeingRegenerated, TEXT("RF_BeingRegenerated"));
				AddFlag(RF_DefaultSubObject, TEXT("RF_DefaultSubObject"));
				AddFlag(RF_WasLoaded, TEXT("RF_WasLoaded"));
				AddFlag(RF_TextExportTransient, TEXT("RF_TextExportTransient"));
				AddFlag(RF_LoadCompleted, TEXT("RF_LoadCompleted"));
				AddFlag(RF_InheritableComponentTemplate, TEXT("RF_InheritableComponentTemplate"));
				AddFlag(RF_DuplicateTransient, TEXT("RF_DuplicateTransient"));
				AddFlag(RF_StrongRefOnFrame, TEXT("RF_StrongRefOnFrame"));
				AddFlag(RF_NonPIEDuplicateTransient, TEXT("RF_NonPIEDuplicateTransient"));
				AddFlag(RF_WillBeLoaded, TEXT("RF_WillBeLoaded"));
				AddFlag(RF_HasExternalPackage, TEXT("RF_HasExternalPackage"));
				AddFlag(RF_MirroredGarbage, TEXT("RF_MirroredGarbage"));
				AddFlag(RF_AllocatedInSharedPage, TEXT("RF_AllocatedInSharedPage"));
			}

			FText PathHint = FText::FromString(Package->GetName());
			FText NameHint = FText::FromString(Object->GetName());
			FText FlagHint = FText::FromString(Out);
			FText HintText = FText::Format(
				FText::FromString("Package: {0}\nObject: {1}\nFlags: {2}"),
				MoveTemp(PathHint),
				MoveTemp(NameHint),
				MoveTemp(FlagHint)
			);
			HeadHintText->SetText(MoveTemp(HintText));
		}
	}
	else
	{
		HeadHintText->SetText(FText::FromString("Select asset from content browser or package from package browser"));
	}
}
