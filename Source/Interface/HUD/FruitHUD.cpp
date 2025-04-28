#include "FruitHUD.h"
#include "Engine/Canvas.h"
#include "Blueprint/UserWidget.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/UI/TitleLevel/TitleLevelWidget.h"
#include "EngineUtils.h"

AFruitHUD::AFruitHUD()
{
    TextureWidget = nullptr;
}

void AFruitHUD::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("FruitHUD: BeginPlay 완료"));

    // 2D 텍스쳐 위젯 그릴 위젯 생성
    CreateAndAddWidgets();

    // TitleLevel이면 타이틀 이미지 세팅
    UWorld* World = GetWorld();
    if (World && World->GetMapName().Contains(TEXT("TitleLevel")))
    {
        if (TextureWidget)
        {
            TextureWidget->SetupTitleImages();
        }

        // TitleLevelWidget 찾아서 초기화 함수 호출
        for (TObjectIterator<UUserWidget> It; It; ++It)
        {
            if (It->IsA<UTitleLevelWidget>() && It->IsInViewport())
            {
                UTitleLevelWidget* TitleWidget = Cast<UTitleLevelWidget>(*It);
                if (TitleWidget)
                {
                    TitleWidget->InitializeTitleWidget();
                    break;
                }
            }
        }
    }
}

void AFruitHUD::CreateAndAddWidgets()
{
    APlayerController* Controller = GetOwningPlayerController();
    if (Controller)
    {
        // 기존 위젯 제거
        if (TextureWidget)
        {
            TextureWidget->RemoveFromParent();
            TextureWidget = nullptr;
        }
        
        // UIWidgetRenderer 생성
        TextureWidget = CreateWidget<UUIWidgetRenderer>(Controller, UUIWidgetRenderer::StaticClass());
        if (TextureWidget)
        {
            // 뷰포트에 추가
            TextureWidget->AddToViewport(9999); // HUD보다 아래에 위치하도록
            
            // 로그
            UE_LOG(LogTemp, Warning, TEXT("FruitHUD: UIWidgetRenderer 생성 및 뷰포트 추가 완료"));
            
            // UMG 렌더링 강제 설정 (아래 함수가 UI 그리는 공간 만드는 핵심)
            if (GEngine && GEngine->GameViewport)
            {
                // 뷰포트 설정 강제 적용
                //GEngine->GameViewport->SetForceDisableSplitscreen(true);

                UE_LOG(LogTemp, Warning, TEXT("GameViewport 검사: %s"), 
                      GEngine->GameViewport->IsValidLowLevel() ? TEXT("유효함") : TEXT("유효하지 않음"));
                
                // Z 순서 최상위로 설정
                TextureWidget->RemoveFromParent();
                GEngine->GameViewport->AddViewportWidgetContent(
                    SNew(SBox)
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Fill)
                    [
                        TextureWidget->TakeWidget()
                    ]
                );
                UE_LOG(LogTemp, Warning, TEXT("SBox에 래핑한 위젯을 뷰포트에 직접 추가"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FruitHUD: UIWidgetRenderer 생성 실패"));
        }
    }
}