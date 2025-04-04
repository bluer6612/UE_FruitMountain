#include "FruitHUD.h"
#include "Engine/Canvas.h"
#include "Blueprint/UserWidget.h"
#include "Interface/UI/SimpleTextureWidget.h"

AFruitHUD::AFruitHUD()
{
    TextureWidget = nullptr;
}

void AFruitHUD::BeginPlay()
{
    Super::BeginPlay();
    
    // 2D 텍스쳐 위젯 그릴 SBOX 래핑 위젯 생성
    CreateAndAddWidgets();
}

void AFruitHUD::CreateAndAddWidgets()
{
    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        // 기존 위젯 제거
        if (TextureWidget)
        {
            TextureWidget->RemoveFromParent();
            TextureWidget = nullptr;
        }
        
        // SimpleTextureWidget 생성
        TextureWidget = CreateWidget<USimpleTextureWidget>(PC, USimpleTextureWidget::StaticClass());
        if (TextureWidget)
        {
            // 뷰포트에 추가
            TextureWidget->AddToViewport(9999); // HUD보다 아래에 위치하도록
            
            // 로그 추가
            UE_LOG(LogTemp, Warning, TEXT("FruitHUD: SimpleTextureWidget 생성 및 뷰포트 추가 완료"));
            
            // 색상 블록 설정 (명시적 호출)
            TextureWidget->SetupAllImages();
            
            // 추가: UMG 렌더링 강제 설정
            if (GEngine && GEngine->GameViewport)
            {
                // 뷰포트 설정 강제 적용 (주석 처리 해도 무관)
                //GEngine->GameViewport->SetForceDisableSplitscreen(true);

                UE_LOG(LogTemp, Warning, TEXT("GameViewport 검사: %s"), 
                      GEngine->GameViewport->IsValidLowLevel() ? TEXT("유효함") : TEXT("유효하지 않음"));
                
                //아래 코드가 2D 텍스쳐 그리는 핵심
                if (TextureWidget)
                {
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
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FruitHUD: SimpleTextureWidget 생성 실패"));
        }
    }
}