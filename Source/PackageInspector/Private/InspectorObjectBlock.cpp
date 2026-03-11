#include "InspectorObjectBlock.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "UObject/UObjectIterator.h"
#include "HAL/PlatformApplicationMisc.h"

//#include "ClassViewerModule.h"
//#include "SClassViewer.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Widgets/Input/STextEntryPopup.h"

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
			//.Text(FText::FromString(
			//	Obj ? Obj->GetName() : TEXT("-")));
			.Text_Lambda([I = Item]()
			{
				if (!I.IsValid()) return FText::FromString("-");
				return FText::FromString(I.Get()->GetName());
			});

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
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				.Padding(5.f,0.f)
				[
					SAssignNew(HeadHintTextLeft, STextBlock)
				]
				
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(5.f,0.f)
				[
					SAssignNew(HeadHintTextRight, STextBlock)
					.Justification(ETextJustify::Right)
				]
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
			.OnExpansionChanged(this, &SInspectorObjectBlock::OnItemExpansionChanged)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("Class").DefaultLabel(FText::FromString("Class"))
				+ SHeaderRow::Column("Name").DefaultLabel(FText::FromString("Name"))
				+ SHeaderRow::Column("Path").DefaultLabel(FText::FromString("Path"))
			)
		]
	];

	UpdateLayout();
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
		ExtractPackageObjects(RootNode, 1);
	}
	UpdateLayout();
}

void SInspectorObjectBlock::RemoveRootObjects(const TArray<UObject*>& RootObject)
{
	for (UObject* Obj : RootObject)
		if (RootItems.Contains(Obj)) RootItems.Remove(Obj);
	UpdateLayout();
}

PRAGMA_DISABLE_OPTIMIZATION
void SInspectorObjectBlock::UpdateLayout()
{
	bool bSkipDuplicateCheck = true;	// set false to debug
	
	TSet<FInspectObjectPtr> Expanded;
	TreeView->GetExpandedItems(Expanded);
	for (FInspectObjectPtr Node : Expanded)
	{
		if (!Node.IsValid())
			continue;
		auto Obj = Node.Get();
		ExtractPackageObjects(Node, 1);
	}

	auto GetDuplicates = [&]() -> TSet<FInspectObjectPtr>
	{
		TSet<FInspectObjectPtr> Unique;
		TSet<FInspectObjectPtr> Duplicates;
		for (auto& ChildSet : ChildrenMap)
		{
			for (auto& Child : ChildSet.Value)
			{
				if (!Unique.Contains(Child))
					Unique.Add(Child);
				else
				{
					Duplicates.Add(Child);
				}
			}
		}
		return Duplicates;
	};
	
	if (!bSkipDuplicateCheck)
	{
		TSet<FInspectObjectPtr> Dup = GetDuplicates();
		if (!Dup.IsEmpty())
		{
			for (auto& Obj : Dup)
			{
				UE_LOG(LogPackageInspector, Warning, TEXT("Duplicate object %s"), *Obj->GetName());
			}
			return;
		}
	}
	
	if (TreeView.IsValid())
	{
		TreeView->RequestTreeRefresh();
	}

	UpdateHint();
}
PRAGMA_ENABLE_OPTIMIZATION

void SInspectorObjectBlock::ExtractPackageObjects(FInspectObjectPtr RootNode, uint8_t depth)
{
	if (!RootNode.IsValid() || !RootNode.Get()->IsValidLowLevel())
	{
		RootItems.Remove(RootNode);
		ChildrenMap.Remove(RootNode);
		return;
	}
	
	UObject* RootObject = RootNode.Get();
	TArray<UObject*> Objects;
	GetObjectsWithOuter(RootObject, Objects, false);

	for (UObject* Obj : Objects)
	{
		FInspectObjectPtr Node (Obj);
		ChildrenMap.FindOrAdd(RootNode).Add(Node);
		if (depth) ExtractPackageObjects(Node, depth - 1);
	}

	if (TSet<FInspectObjectPtr>* Childs = ChildrenMap.Find(RootNode))
	{
		for (auto It = Childs->CreateIterator(); It; ++It)
		{
			if (!It->IsValid() || It->Get()->GetOuter() != RootObject)
			{
				It.RemoveCurrent();
			}
		}
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

void SInspectorObjectBlock::OnRenameCommitted(const FText& Text, ETextCommit::Type Arg, UObject* Object)
{
	if (Arg != ETextCommit::OnEnter || !Object)
		return;

	FString NewName = Text.ToString();
	Object->Rename(*NewName);
	
	FSlateApplication::Get().DismissAllMenus();
}

void SInspectorObjectBlock::RenameSelectedObject()
{
	if (!TreeView || TreeView->GetNumItemsSelected() != 1) return;
	UObject* Selected = TreeView->GetSelectedItems()[0].Get();
	if (!Selected) return;
	
	TSharedRef<STextEntryPopup> TextEntry =
		SNew(STextEntryPopup)
		.Label(FText::FromString("New Name"))
		.DefaultText(FText::FromName(Selected->GetFName()))
		.OnTextCommitted(FOnTextCommitted::CreateSP(this, &SInspectorObjectBlock::OnRenameCommitted, Selected));

	FSlateApplication::Get().PushMenu(
		SharedThis(this),
		FWidgetPath(),
		TextEntry,
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup)
	);
}

void SInspectorObjectBlock::OnRemoveFromPackage()
{
	auto Selected = TreeView->GetSelectedItems();

	for (auto& Item : Selected)
	{
		if (!Item.IsValid() || !Item.Get()) 
			continue;

		UObject* Obj = Item.Get();

		Obj->RemoveFromRoot();
		Obj->Rename(nullptr, GetTransientPackage());
		//Obj->MarkAsGarbage();
	}

	UpdateLayout();
}

TSharedPtr<SWidget> SInspectorObjectBlock::OnContextMenuOpening()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Copy To Clipboard"),
		FText::FromString("Copy selected package paths"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this,
				&SInspectorObjectBlock::CopySelectionToClipboard)
		)
	);
	
	if (TreeView->GetSelectedItems().Num() == 1)
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Rename"),
			FText::FromString("Rename this object without redirection"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SInspectorObjectBlock::RenameSelectedObject))
		);
		
		MenuBuilder.AddMenuEntry(
			FText::FromString("Create SubObject"),
			FText::FromString("Create new UObject with this object as Outer"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SInspectorObjectBlock::CreateSubObject)
			)
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Remove from Package"),
			FText::FromString("Remove the selected object from its package (runtime only)"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SInspectorObjectBlock::OnRemoveFromPackage)
			)
		);
	}

	return MenuBuilder.MakeWidget();
}

void SInspectorObjectBlock::CopySelectionToClipboard()
{
	//TArray<FInspectObjectPtr> Selected;
	//TreeView->GetSelectedItems(Selected);

	FString Result;

	// probably there is only one selected item, but still
	for (FInspectObjectPtr Item : TreeView->GetSelectedItems())
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

void SInspectorObjectBlock::CreateSubObject()
{
	UClass* ChosenClass = nullptr;

	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.bShowNoneOption = false;
	Options.bShowUnloadedBlueprints = true;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::ClassName;

	const bool bPressedOk =
		SClassPickerDialog::PickClass(
			FText::FromString("Choose Object Class"),
			Options,
			ChosenClass,
			UObject::StaticClass()
		);

	if (!bPressedOk || !ChosenClass)
	{
		return;
	}

	FText DefaultName = FText::FromString("NewObject");

	TSharedRef<STextEntryPopup> TextEntry =
		SNew(STextEntryPopup)
		.Label(FText::FromString("Object Name"))
		.DefaultText(DefaultName)
		.OnTextCommitted(
			FOnTextCommitted::CreateSP(
				this,
				&SInspectorObjectBlock::OnNewSubObjectNameCommitted,
				ChosenClass
			)
		);

	FSlateApplication::Get().PushMenu(
	SharedThis(this),
	FWidgetPath(),
	TextEntry,
	FSlateApplication::Get().GetCursorPos(),
	FPopupTransitionEffect::TypeInPopup
	);

	
}


void SInspectorObjectBlock::OnNewSubObjectNameCommitted(const FText& Text, ETextCommit::Type Arg, UClass* Class)
{
	if (Arg != ETextCommit::OnEnter)
		return;

	FString NameString = Text.ToString();

	UObject* Outer = nullptr;
	if (TreeView)
	{
		Outer = TreeView->GetSelectedItems()[0].Get();
	}
	if (!Outer) return;
	
	UObject* NewObj = NewObject<UObject>(
		Outer,
		Class,
		*NameString,
		RF_Transient
	);

	if (NewObj)
	{
		NewObj->AddToRoot(); // GC remark
	}
	
	FSlateApplication::Get().DismissAllMenus();
	UpdateLayout();
	TreeView->SetItemExpansion(Outer, true);
	TreeView->SetSelection(NewObj);
	UpdateLayout();
}

auto SInspectorObjectBlock::OnItemExpansionChanged(FInspectObjectPtr Item, bool bExpanded) -> void
{
	if (bExpanded)
	{
		UpdateLayout();
	}
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
				FText::FromString("{0}\n{1}\n{2}"),
				MoveTemp(PathHint),
				MoveTemp(NameHint),
				MoveTemp(FlagHint)
			);
			HeadHintTextLeft->SetText(FText::FromString("Package:\nObject:\nFlags:"));
			HeadHintTextRight->SetText(MoveTemp(HintText));
		}
	}
	else
	{
		HeadHintTextLeft->SetText(FText::FromString("Select asset from content browser or package from package browser"));
		HeadHintTextRight->SetText(FText());
	}
}