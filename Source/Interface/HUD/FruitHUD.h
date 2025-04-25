#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Interface/UI/UIWidgetRenderer.h"
#include "FruitHUD.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitHUD : public AHUD
{
    GENERATED_BODY()
    
public:
    AFruitHUD();
    
    virtual void BeginPlay() override;
    
protected:
    // UMG 위젯 참조
    UPROPERTY()
    UUIWidgetRenderer* TextureWidget;
    
    // 위젯 생성 함수
    void CreateAndAddWidgets();
    
private:
    // 모든 활성 위젯 찾기
    TArray<UUserWidget*> GetAllActiveWidgets();
    
    // 재귀적으로 위젯 수집
    void CollectWidgetsFromWidget(UWidget* Widget, TArray<UUserWidget*>& OutWidgets);
};