#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FruitHUD.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitHUD : public AHUD
{
    GENERATED_BODY()
    
public:
    AFruitHUD();
    
    virtual void DrawHUD() override;
    
    // 빨간색 사각형 그리기
    void DrawRedSquare();
    
private:
    // 사각형 크기
    float SquareSize;
    
    // 사각형 색상
    FLinearColor SquareColor;
    
    // 사각형 위치 (화면 비율)
    FVector2D SquarePosition;
};