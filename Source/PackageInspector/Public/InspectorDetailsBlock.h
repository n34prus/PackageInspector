#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "IDetailsView.h"

class SDirtyIndicator : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDirtyIndicator) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void BindObject(UObject* Object);
	void UpdateLayout();

private:
	
	EActiveTimerReturnType OnTick(double InCurrentTime, float InDeltaTime);
	FReply OnMarkAsDirtyPressed();
	bool IsOutermostDirty() const;
	void MarkOutermostAsDirty();
	
	TSharedPtr<SImage> DirtyIcon;
	TSharedPtr<SButton> DirtyButton;
	TSharedPtr<STextBlock> DirtyText; 
	TWeakObjectPtr<UObject> ObjectPtr;
};

class SInspectorDetailsBlock : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorDetailsBlock) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateLayout();

	void SetObject(UObject* Object);
	void SetEditingEnabled(bool bEnabled);

private:

	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SDirtyIndicator> DirtyIndicator;
};

