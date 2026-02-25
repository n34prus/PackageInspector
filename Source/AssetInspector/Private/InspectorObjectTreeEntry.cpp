#include "InspectorObjectTreeEntry.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Brushes/SlateImageBrush.h"
#include "Engine/Texture2D.h"

TSharedRef<SWidget> UInspectorObjectTreeEntry::RebuildWidget()
{
	SAssignNew(RootBox, SVerticalBox)

	+ SVerticalBox::Slot().AutoHeight()
	[
		SAssignNew(IconWidget, SImage)
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SAssignNew(ClassText, STextBlock)
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SAssignNew(StatusText, STextBlock)
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SAssignNew(NameText, STextBlock)
	]

	+ SVerticalBox::Slot().AutoHeight()
	[
		SAssignNew(PathText, STextBlock)
	];

	return RootBox.ToSharedRef();
}

void UInspectorObjectTreeEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	SourceObject = ListItemObject;
	UpdateStatus();
	RefreshData();
}

void UInspectorObjectTreeEntry::UpdateStatus()
{
	if (!IsValid(SourceObject))
	{
		Status = EAssetInspectorStatus::Status_Bad;
	}
	else
	{
		Status = EAssetInspectorStatus::Status_Good;
	}
}

void UInspectorObjectTreeEntry::RefreshData()
{
	if (!RootBox.IsValid())
		return;

	if (!SourceObject)
	{
		ClassText->SetText(FText::FromString("None"));
		StatusText->SetText(FText::FromString("Bad"));
		NameText->SetText(FText::FromString("None"));
		PathText->SetText(FText::FromString("None"));
		return;
	}

	ClassText->SetText(FText::FromString(SourceObject->GetClass()->GetName()));
	NameText->SetText(FText::FromString(SourceObject->GetName()));
	PathText->SetText(FText::FromString(SourceObject->GetPathName()));
	StatusText->SetText(FText::FromString(UEnum::GetValueAsString(Status)));

	if (IconWidget.IsValid() && IconSoft.IsValid())
	{
		UTexture2D* Tex = IconSoft.LoadSynchronous();

		if (Tex)
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(Tex);
			IconWidget->SetImage(&Brush);
		}
	}
}