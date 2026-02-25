#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_ThreeParams(
	FOnInspectorSettingsChanged,
	bool /*bAllPackages*/,
	bool /*bIncludeTransient*/,
	bool /*bAllowEditing*/
);

class SInspectorSettingsBlock : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInspectorSettingsBlock) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	bool IsAllPackages() const { return bAllPackages; }
	bool IsIncludeTransient() const { return bIncludeTransient; }
	bool IsAllowEditing() const { return bAllowEditing; }

	FOnInspectorSettingsChanged OnSettingsChanged;

private:

	bool bAllPackages = false;
	bool bIncludeTransient = false;
	bool bAllowEditing = true;

	void OnSettingChanged();
};