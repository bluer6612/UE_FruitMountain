#include "FruitHUD.h"
#include "Engine/Canvas.h"

AFruitHUD::AFruitHUD()
{
    // 기본값 설정
    SquareSize = 300.0f;
    SquareColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);  // 빨간색
    SquarePosition = FVector2D(0.5f, 0.5f);  // 화면 중앙
}

void AFruitHUD::DrawHUD()
{
    Super::DrawHUD();
    
    // 항상 빨간색 사각형 그리기
    DrawRedSquare();
}

void AFruitHUD::DrawRedSquare()
{
    if (Canvas)
    {
        // 화면 크기 가져오기
        const float ScreenWidth = Canvas->ClipX;
        const float ScreenHeight = Canvas->ClipY;
        
        // 화면 중앙 위치 계산
        const float CenterX = ScreenWidth * SquarePosition.X;
        const float CenterY = ScreenHeight * SquarePosition.Y;
        
        // 사각형 왼쪽 위 좌표 계산
        const float Left = CenterX - (SquareSize * 0.5f);
        const float Top = CenterY - (SquareSize * 0.5f);
        
        // 사각형 그리기
        FCanvasTileItem TileItem(FVector2D(Left, Top), 
                                FVector2D(SquareSize, SquareSize), 
                                SquareColor);
        TileItem.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(TileItem);
        
        UE_LOG(LogTemp, Error, TEXT("HUD: 빨간색 사각형 그려짐 - 위치: (%f, %f), 크기: %f"), Left, Top, SquareSize);
    }
}