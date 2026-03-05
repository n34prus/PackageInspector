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
		[
			SAssignNew(DirtyIcon, SImage)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(4)
		[
			SAssignNew(DirtyButton, SButton)
			.Text(FText::FromString("MarkPackageDirty"))
			.OnClicked(this, &SDirtyIndicator::OnMarkAsDirtyPressed)
		]
	];
	UpdateVisuals();
}

void SDirtyIndicator::BindObject(UObject* Object)
{
	this->ObjectPtr = Object;
	UpdateVisuals();
	if (ObjectPtr.IsValid())
	{
		TimerHandlePtr = RegisterActiveTimer(1.0f, FWidgetActiveTimerDelegate::CreateSP(this, &SDirtyIndicator::OnTick));
	}
	else
	{
		if (TimerHandlePtr.IsValid())
		{
			UnRegisterActiveTimer(TimerHandlePtr.ToSharedRef());
		}
	}
}

EActiveTimerReturnType SDirtyIndicator::OnTick(double InCurrentTime, float InDeltaTime)
{
	UpdateVisuals();
	return EActiveTimerReturnType::Continue;
}

void SDirtyIndicator::UpdateVisuals()
{
	const bool bIsDirty = IsOutermostDirty();

	if (DirtyIcon)
	{
		if (ObjectPtr.IsValid())
		{
			DirtyIcon->SetEnabled(true);
			DirtyButton->SetEnabled(!bIsDirty);
			DirtyIcon->SetImage(
				bIsDirty
					? FAppStyle::GetBrush("Icons.Save")
					: FAppStyle::GetBrush("Icons.Check")
			);
		}
		else
		{
			DirtyIcon->SetEnabled(false);
			DirtyButton->SetEnabled(false);
		}
	}
}

FReply SDirtyIndicator::OnMarkAsDirtyPressed()
{
	MarkOutermostAsDirty();
	UpdateVisuals();
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