// 새 파일: BasicHUD.cpp
#include "BasicHUD.h"
#include "Engine/Canvas.h"

void ABasicHUD::DrawHUD()
{
    Super::DrawHUD();
    
    // 화면 중앙에 큰 빨간 사각형 그리기
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
        
        // 텍스트 그리기
        FString Text = TEXT("HUD에 직접 그린 UI");
        FCanvasTextItem TextItem(FVector2D(X + 20, Y + 20), FText::FromString(Text), GEngine->GetLargeFont(), FLinearColor::White);
        Canvas->DrawItem(TextItem);
    }
}