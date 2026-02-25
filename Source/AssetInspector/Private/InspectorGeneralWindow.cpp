#include "InspectorGeneralWindow.h"
#include "InspectorSettingsBlock.h"
#include "InspectorTreeView.h"
#include "InspectorDetailsBlock.h"

#include "Widgets/Layout/SSplitter.h"

void SInspectorGeneralWindow::Construct(const FArguments& InArgs)
{
	RebuildLayout();
	
}

void SInspectorGeneralWindow::RebuildLayout()
{
	ChildSlot
	[
		SNew(SSplitter)

		+ SSplitter::Slot()
		.Value(0.25f)
		[
			SAssignNew(SettingsBlock, SInspectorSettingsBlock)
		]

		+ SSplitter::Slot()
		.Value(0.45f)
		[
			SAssignNew(TreeBlock, SInspectorTreeView)
		]

		+ SSplitter::Slot()
		.Value(0.30f)
		[
			SAssignNew(DetailsBlock, SInspectorDetailsBlock)
		]
	];

	SettingsBlock->OnSettingsChanged.BindLambda(
[this](bool bAll, bool bTransient, bool bEdit)
{
	//TreeBlock->SetMode(bAll, bTransient);
	DetailsBlock->SetEditingEnabled(bEdit);
});

	TreeBlock->OnObjectSelected.BindLambda(
[this](UObject* Obj)
{
	if (DetailsBlock)
	{
		DetailsBlock->SetObject(Obj);
	}
});
}