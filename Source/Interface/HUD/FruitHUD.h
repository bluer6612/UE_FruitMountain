#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FruitHUD.generated.h"

class UUIWidgetRenderer;
class UTitleLevelWidget;

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitHUD : public AHUD
{
    GENERATED_BODY()

public:
    AFruitHUD();

    virtual void BeginPlay() override;
    
    // 필요한 위젯 생성 및 추가
    void CreateAndAddWidgets();
    
    // TitleWidget 제거 함수
    void ClearTitleWidget();
    
    // TitleWidget 설정 함수
    void SetTitleWidget(UTitleLevelWidget* InTitleWidget) { TitleWidget = InTitleWidget; }

    // TextureWidget 접근자 함수 추가
    UUIWidgetRenderer* GetTextureWidget() const { return TextureWidget; }

protected:
    // 위젯 컴포넌트
    UPROPERTY()
    UUIWidgetRenderer* TextureWidget;
    
    // 타이틀 위젯 저장용 변수
    UPROPERTY()
    UTitleLevelWidget* TitleWidget;
};