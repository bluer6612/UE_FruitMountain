#include "TitleGameMode.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Interface/HUD/FruitHUD.h"

ATitleGameMode::ATitleGameMode()
{
    HUDClass = AFruitHUD::StaticClass();

    // 블루프린트 클래스 참조 - 정확한 경로 지정
    static ConstructorHelpers::FClassFinder<UMainMenuWidget> MainMenuWidgetClassFinder(TEXT("/Game/UI/TitleLevel/BP_MainMenuWidget"));
    if (MainMenuWidgetClassFinder.Succeeded())
    {
        MainMenuWidgetClass = MainMenuWidgetClassFinder.Class;
        UE_LOG(LogTemp, Warning, TEXT("BP_MainMenuWidget 클래스 로드 성공"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BP_MainMenuWidget 클래스를 찾을 수 없음!"));
    }
}

void ATitleGameMode::BeginPlay()
{
    Super::BeginPlay();

    FString MapName = GetWorld()->GetMapName();
    if (!MapName.Contains(TEXT("TitleLevel")))
    {
        UE_LOG(LogTemp, Warning, TEXT("TitleGameMode: 현재 맵이 TitleLevel이 아니므로 MainMenuWidget을 생성하지 않습니다. (MapName=%s)"), *MapName);
        return;
    }

    UWorld* World = GetWorld();
    if (World && MainMenuWidgetClass)
    {
        MainMenuWidget = CreateWidget<UMainMenuWidget>(World, MainMenuWidgetClass);
        if (MainMenuWidget)
        {
            MainMenuWidget->AddToViewport();

            // HUD에게 MainMenuWidget 인스턴스 전달
            if (AFruitHUD* FruitHUD = Cast<AFruitHUD>(World->GetFirstPlayerController()->GetHUD()))
            {
                UE_LOG(LogTemp, Warning, TEXT("TitleGameMode: SetMainMenuWidget 호출, MainMenuWidget=%p"), MainMenuWidget);
                FruitHUD->SetMainMenuWidget(MainMenuWidget);
            }

            // 입력 비활성화
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                PC->SetIgnoreMoveInput(true);
                PC->SetIgnoreLookInput(true);
            }
        }
    }
}