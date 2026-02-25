#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "InspectorObjectTreeEntry.generated.h"

class SVerticalBox;
class SImage;
class STextBlock;

UENUM()
enum class EAssetInspectorStatus : uint8
{
	Status_Unknown,
	Status_Bad,
	Status_System,
	Status_Good
};

UCLASS()
class ASSETINSPECTOR_API UInspectorObjectTreeEntry
	: public UUserWidget
	, public IUserObjectListEntry
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category="AssetInspector")
	TSoftObjectPtr<UTexture2D> IconSoft;

	UPROPERTY(BlueprintReadOnly, Category="AssetInspector")
	UObject* SourceObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="AssetInspector")
	EAssetInspectorStatus Status = EAssetInspectorStatus::Status_Unknown;
	
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual TSharedRef<SWidget> RebuildWidget() override;

	void UpdateStatus();
	void RefreshData();

private:

	TSharedPtr<SVerticalBox> RootBox;
	TSharedPtr<SImage> IconWidget;
	TSharedPtr<STextBlock> ClassText;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> NameText;
	TSharedPtr<STextBlock> PathText;
};