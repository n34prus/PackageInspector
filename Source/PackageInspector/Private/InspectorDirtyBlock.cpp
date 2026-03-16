#include "InspectorDirtyBlock.h"

void SInspectorDirtyStatusBlock::Construct(const FArguments& InArgs)
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
			.OnClicked(this, &SInspectorDirtyStatusBlock::OnMarkAsDirtyPressed)
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

void SInspectorDirtyStatusBlock::BindObject(UObject* Object)
{
	this->ObjectPtr = Object;
	UpdateLayout();
}

void SInspectorDirtyStatusBlock::UpdateLayout()
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

FReply SInspectorDirtyStatusBlock::OnMarkAsDirtyPressed()
{
	MarkOutermostAsDirty();
	UpdateLayout();
	return FReply::Handled();
}

bool SInspectorDirtyStatusBlock::IsOutermostDirty() const
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

void SInspectorDirtyStatusBlock::MarkOutermostAsDirty()
{
	if (ObjectPtr.IsValid())
	{
		if (UObject* Obj = ObjectPtr.Get())
		{
			Obj->MarkPackageDirty();
		}
	}
}