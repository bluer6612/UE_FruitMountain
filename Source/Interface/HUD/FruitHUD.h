#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FruitHUD.generated.h"

class UUIWidgetRenderer;
class UMainMenuWidget;

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitHUD : public AHUD
{
    GENERATED_BODY()
public:
    AFruitHUD();

    virtual void BeginPlay() override;

    void CreateAndAddWidgets();

    void SetMainMenuWidget(UMainMenuWidget* InWidget) { MainMenuWidget = InWidget; }
    UMainMenuWidget* GetMainMenuWidget() const { return MainMenuWidget; }
    UUIWidgetRenderer* GetTextureWidget() const { return TextureWidget; }
protected:
    UPROPERTY()
    UUIWidgetRenderer* TextureWidget;

    UPROPERTY()
    UMainMenuWidget* MainMenuWidget;
};