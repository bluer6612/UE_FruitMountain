#include "TitleGameMode.h"
#include "Interface/UI/TitleLevel/TitleLevelWidget.h"

ATitleGameMode::ATitleGameMode()
{
    // 필요 시 기본값 설정
}

void ATitleGameMode::BeginPlay()
{
    Super::BeginPlay();

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