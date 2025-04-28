#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/UI/TitleLevel/TitleLevelWidget.h"
#include "FruitHUD.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitHUD : public AHUD
{
    GENERATED_BODY()
    
public:
    AFruitHUD();
    
    virtual void BeginPlay() override;

    UUIWidgetRenderer* GetTextureWidget() const
    {
        return TextureWidget;
    }

    void SetTitleWidget(UTitleLevelWidget* InTitleWidget) { CachedTitleWidget = InTitleWidget; }
    
protected:
    // UMG 위젯 참조
    UPROPERTY()
    UUIWidgetRenderer* TextureWidget;
    
    // 위젯 생성 함수
    void CreateAndAddWidgets();

private:
    UTitleLevelWidget* CachedTitleWidget = nullptr;
};