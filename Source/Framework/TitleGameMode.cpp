#include "TitleGameMode.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Interface/HUD/FruitHUD.h"

ATitleGameMode::ATitleGameMode()
{
    HUDClass = AFruitHUD::StaticClass();

    // 블루프린트 클래스 참조 - 정확한 경로 지정
    static ConstructorHelpers::FClassFinder<UMainMenuWidget> TitleWidgetClassFinder(TEXT("/Game/UI/TitleLevel/BP_TitleLevelWidget"));
    
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

    FString MapName = GetWorld()->GetMapName();
    if (!MapName.Contains(TEXT("TitleLevel"))) // 반드시 TitleLevel에서만 생성
    {
        UE_LOG(LogTemp, Warning, TEXT("TitleGameMode: 현재 맵이 TitleLevel이 아니므로 MainMenuWidget을 생성하지 않습니다. (MapName=%s)"), *MapName);
        return;
    }
    
    // 현재 레벨이 TitleLevel인 경우에만 TitleWidget 생성
    UWorld* World = GetWorld();
    if (World && World->GetMapName().Contains(TEXT("TitleLevel")))
    {
        if (TitleWidgetClass)
        {
            TitleWidget = CreateWidget<UMainMenuWidget>(GetWorld(), TitleWidgetClass);
            if (TitleWidget)
            {
                TitleWidget->AddToViewport();

                // HUD에게 TitleWidget 인스턴스 전달
                if (AFruitHUD* FruitHUD = Cast<AFruitHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
                {
                    UE_LOG(LogTemp, Warning, TEXT("TitleGameMode: SetTitleWidget 호출, TitleWidget=%p"), TitleWidget);
                    FruitHUD->SetTitleWidget(TitleWidget);
                }

                // 입력 비활성화: 플레이어 조종 막기
                if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
                {
                    PC->SetIgnoreMoveInput(true);
                    PC->SetIgnoreLookInput(true);
                }
            }
        }
    }
}