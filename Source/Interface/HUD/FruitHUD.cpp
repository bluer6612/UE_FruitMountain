#include "FruitHUD.h"
#include "Engine/Canvas.h"
#include "Blueprint/UserWidget.h"
#include "Interface/UI/UIWidgetRenderer.h"

AFruitHUD::AFruitHUD()
{
    TextureWidget = nullptr;
}

void AFruitHUD::BeginPlay()
{
    Super::BeginPlay();
    
    // 글로벌 폰트 설정
    UUIWidgetUtility::SetGlobalFont();
    
    // 2D 텍스쳐 위젯 그릴 위젯 생성
    CreateAndAddWidgets();
    
    // 약간의 지연 후 모든 텍스트 위젯에 폰트 적용
    FTimerHandle FontApplyHandle;
    GetWorldTimerManager().SetTimer(FontApplyHandle, [this]()
    {
        // 뷰포트의 모든 위젯 탐색
        if (GEngine && GEngine->GameViewport)
        {
            TArray<UUserWidget*> AllWidgets;
            // 모든 활성 위젯 찾기 (별도 함수로 구현 필요)
            // ...
            
            // 각 위젯에 폰트 적용
            for (UUserWidget* Widget : AllWidgets)
            {
                UUIWidgetUtility::ApplyFontToAllTextBlocks(Widget, TEXT(""), 0);
            }
        }
    }, 0.5f, false); // 0.5초 후 실행
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
            
            // 이미지 설정 함수 호출
            TextureWidget->SetupAllImages();
            
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