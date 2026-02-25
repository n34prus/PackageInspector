#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SInspectorSettingsBlock;
class SInspectorTreeView;
class SInspectorDetailsBlock;

class SInspectorGeneralWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorGeneralWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	
	TSharedPtr<SInspectorSettingsBlock> SettingsBlock;
	TSharedPtr<SInspectorTreeView>      TreeBlock;
	TSharedPtr<SInspectorDetailsBlock>  DetailsBlock;
	
	void RebuildLayout();
};