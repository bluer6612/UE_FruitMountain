#include "TitleGameMode.h"
#include "Interface/UI/TitleLevel/TitleLevelWidget.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Interface/HUD/FruitHUD.h"

ATitleGameMode::ATitleGameMode()
{
    HUDClass = AFruitHUD::StaticClass();

    // 블루프린트 클래스 참조 - 정확한 경로 지정
    static ConstructorHelpers::FClassFinder<UTitleLevelWidget> TitleWidgetClassFinder(TEXT("/Game/UI/TitleLevel/BP_TitleLevelWidget"));
    
    if (TitleWidgetClassFinder.Succeeded())
    {
        TitleWidgetClass = TitleWidgetClassFinder.Class;
        UE_LOG(LogTemp, Warning, TEXT("BP_TitleLevelWidget 클래스 로드 성공"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BP_TitleLevelWidget 클래스를 찾을 수 없음!"));
    }
}

void ATitleGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    if (TitleWidgetClass)
    {
        TitleWidget = CreateWidget<UTitleLevelWidget>(GetWorld(), TitleWidgetClass);
        if (TitleWidget)
        {
            TitleWidget->AddToViewport();
            
            // HUD에게 TitleWidget 인스턴스 전달
            if (AFruitHUD* FruitHUD = Cast<AFruitHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
            {
                FruitHUD->SetTitleWidget(TitleWidget);
            }
        }
    }
}