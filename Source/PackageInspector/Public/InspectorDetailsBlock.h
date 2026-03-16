#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "IDetailsView.h"

class SInspectorDetailsBlock : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorDetailsBlock) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateLayout();

	void BindObject(UObject* Object);
	void SetEditingEnabled(bool bEnabled);

private:

	TSharedPtr<IDetailsView> DetailsView;
};

