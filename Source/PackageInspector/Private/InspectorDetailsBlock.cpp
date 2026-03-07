#include "InspectorDetailsBlock.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

void SDirtyIndicator::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SHorizontalBox)
		
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(5)
		[
			SAssignNew(DirtyButton, SButton)
			.Text(FText::FromString("MarkPackageDirty"))
			.OnClicked(this, &SDirtyIndicator::OnMarkAsDirtyPressed)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.f)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(5)
		[
			SAssignNew(DirtyText, STextBlock)
			.Text(FText::FromString(" - "))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(5)
		[
			SAssignNew(DirtyIcon, SImage)
		]
		
	];
	UpdateLayout();
}

void SDirtyIndicator::BindObject(UObject* Object)
{
	this->ObjectPtr = Object;
	UpdateLayout();
}

EActiveTimerReturnType SDirtyIndicator::OnTick(double InCurrentTime, float InDeltaTime)
{
	UpdateLayout();
	return EActiveTimerReturnType::Continue;
}

void SDirtyIndicator::UpdateLayout()
{
	const bool bIsDirty = IsOutermostDirty();

	if (DirtyIcon)
	{
		if (ObjectPtr.IsValid())
		{
			DirtyIcon->SetVisibility(EVisibility::Visible);
			DirtyButton->SetEnabled(!bIsDirty);
			DirtyIcon->SetImage(
				bIsDirty
				? FAppStyle::GetBrush("Icons.Save")
				: FAppStyle::GetBrush("Icons.Check")
				);
			DirtyText->SetText(
				bIsDirty
				? FText::FromString("DIRTY")
				: FText::FromString("CLEAN")
				);
		}
		else
		{
			DirtyIcon->SetVisibility(EVisibility::Hidden);
			DirtyButton->SetEnabled(false);
			DirtyText->SetText(FText::FromString("NONE"));
		}
	}
}

FReply SDirtyIndicator::OnMarkAsDirtyPressed()
{
	MarkOutermostAsDirty();
	UpdateLayout();
	return FReply::Handled();
}

bool SDirtyIndicator::IsOutermostDirty() const
{
	if (ObjectPtr.IsValid())
	{
		if (UObject* Obj = ObjectPtr.Get())
		{
			if (UPackage* Outer = Obj->GetOutermost())
			{
				return Outer->IsDirty();
			}
		}
	}
	return false;
}

void SDirtyIndicator::MarkOutermostAsDirty()
{
	if (ObjectPtr.IsValid())
	{
		if (UObject* Obj = ObjectPtr.Get())
		{
			Obj->MarkPackageDirty();
		}
	}
}

void SInspectorDetailsBlock::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs Args;
	
	Args.bLockable = false;
	Args.bUpdatesFromSelection = false;
	Args.bAllowSearch = true;
	Args.bShowOptions = true;
	Args.bShowScrollBar = true;
	Args.bShowPropertyMatrixButton = true;
	Args.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	Args.bForceHiddenPropertyVisibility = true;
	Args.bShowHiddenPropertiesWhilePlayingOption = true;
	Args.bAllowFavoriteSystem = true;
	Args.bShowModifiedPropertiesOption = true;

	DetailsView = PropModule.CreateDetailView(Args);

	ChildSlot
	[
		//DetailsView.ToSharedRef()
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			DetailsView.ToSharedRef()
		]
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(50.f)
			.VAlign(VAlign_Center)
			[
				SAssignNew(DirtyIndicator, SDirtyIndicator)
			]
		]
	];
}

void SInspectorDetailsBlock::UpdateLayout()
{
	if (DirtyIndicator) DirtyIndicator->UpdateLayout();
}

void SInspectorDetailsBlock::SetObject(UObject* Object)
{
	if (!Object || !Object->IsValidLowLevel())
	{
		Object = nullptr;
	}
	
	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(Object);
	}
	if (DirtyIndicator.IsValid())
	{
		DirtyIndicator->BindObject(Object);
	}
}

void SInspectorDetailsBlock::SetEditingEnabled(bool bEnabled)
{
	if (DetailsView.IsValid())
	{
		DetailsView->SetIsPropertyEditingEnabledDelegate(
			FIsPropertyEditingEnabled::CreateLambda([bEnabled]()
			{
				return bEnabled;
			}));
	}
}