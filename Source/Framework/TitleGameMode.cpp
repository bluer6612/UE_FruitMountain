#include "TitleGameMode.h"
#include "Interface/UI/TitleLevel/TitleLevelWidget.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/HUD/FruitHUD.h"
#include "Kismet/GameplayStatics.h"

ATitleGameMode::ATitleGameMode()
{
    // TitleLevel에서도 FruitHUD를 사용하도록 명시적으로 설정
    HUDClass = AFruitHUD::StaticClass();
}

void ATitleGameMode::BeginPlay()
{
    Super::BeginPlay();

    // TitleLevel에서도 Renderer 인스턴스 생성
    //UUIWidgetRenderer::CreateDisplayWidget(GetWorld());
    
    // 타이틀 위젯 생성 및 뷰포트에 추가
    if (!TitleWidget)
    {
        TitleWidget = CreateWidget<UTitleLevelWidget>(GetWorld(), UTitleLevelWidget::StaticClass());
        if (TitleWidget)
        {
            TitleWidget->AddToViewport();
            TitleWidget->SetKeyboardFocus();
        }
    }
}