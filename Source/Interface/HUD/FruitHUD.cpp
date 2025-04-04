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
    
    // 위젯 생성 및 추가
    CreateAndAddWidgets();
}

void AFruitHUD::DrawHUD()
{
    Super::DrawHUD();
    
    // HUD에 직접 텍스처 그리기
    if (Canvas)
    {
        float ScreenWidth = Canvas->ClipX;
        float ScreenHeight = Canvas->ClipY;
        
        // 위치 및 크기 계산
        float BoxWidth = 500.0f;
        float BoxHeight = 300.0f;
        float X = (ScreenWidth - BoxWidth) / 2.0f;
        float Y = (ScreenHeight - BoxHeight) / 2.0f;
        
        // 빨간색 사각형 그리기
        FLinearColor RedColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.8f);
        FCanvasTileItem TileItem(FVector2D(X, Y), FVector2D(BoxWidth, BoxHeight), RedColor);
        TileItem.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(TileItem);
        
        // S_Actor 텍스처 직접 그리기
        UTexture2D* ActorTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/S_Actor"));
        if (ActorTexture)
        {
            // 오른쪽 상단에 텍스처 그리기
            FCanvasTileItem TextureItem(
                FVector2D(ScreenWidth - 200.0f, 50.0f), 
                ActorTexture->GetResource(), 
                FVector2D(128.0f, 128.0f), 
                FLinearColor::White
            );
            TextureItem.BlendMode = SE_BLEND_Translucent;
            Canvas->DrawItem(TextureItem);
            
            // 텍스처 그리기 성공 로그
            UE_LOG(LogTemp, Warning, TEXT("HUD에 S_Actor 텍스처 직접 그리기 성공!"));
        }
        
        // 텍스트 그리기
        FString Text = TEXT("HUD에 직접 그린 UI");
        FCanvasTextItem TextItem(FVector2D(X + 20, Y + 20), FText::FromString(Text), GEngine->GetLargeFont(), FLinearColor::White);
        Canvas->DrawItem(TextItem);
        
        // UMG 위젯 상태 표시
        FString WidgetStatus = TextureWidget ? TEXT("UMG 위젯 생성됨") : TEXT("UMG 위젯 NULL");
        FCanvasTextItem StatusItem(FVector2D(X + 20, Y + 60), FText::FromString(WidgetStatus), GEngine->GetLargeFont(), FLinearColor::Yellow);
        Canvas->DrawItem(StatusItem);
    }
}

void AFruitHUD::CreateAndAddWidgets()
{
    // 플레이어 컨트롤러 가져오기
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
            
            // 입력 모드 설정 (중요)
            PC->SetInputMode(FInputModeGameAndUI());
            PC->SetShowMouseCursor(true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FruitHUD: SimpleTextureWidget 생성 실패"));
        }
    }
}