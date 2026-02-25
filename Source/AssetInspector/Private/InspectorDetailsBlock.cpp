#include "InspectorDetailsBlock.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"

void SInspectorDetailsBlock::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs Args;
	Args.bAllowSearch = true;
	Args.bLockable = false;
	Args.bUpdatesFromSelection = false;
	Args.bHideSelectionTip = true;
	Args.bShowOptions = true;

	DetailsView = PropModule.CreateDetailView(Args);

	ChildSlot
	[
		DetailsView.ToSharedRef()
	];
}

void SInspectorDetailsBlock::SetObject(UObject* Object)
{
	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(Object);
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