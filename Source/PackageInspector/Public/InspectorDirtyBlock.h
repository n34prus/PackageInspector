#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SInspectorDirtyStatusBlock : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorDirtyStatusBlock) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void BindObject(UObject* Object);
	void UpdateLayout();

private:
	
	FReply OnMarkAsDirtyPressed();
	bool IsOutermostDirty() const;
	void MarkOutermostAsDirty();
	
	TSharedPtr<SImage> DirtyIcon;
	TSharedPtr<SButton> DirtyButton;
	TSharedPtr<STextBlock> DirtyText; 
	TWeakObjectPtr<UObject> ObjectPtr;
};