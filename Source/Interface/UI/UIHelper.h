#pragma once

#include "CoreMinimal.h"
#include "UIHelper.generated.h"

// 위젯 위치 열거형 - 이미지 변수명과 동기화
UENUM(BlueprintType)
enum class EWidgetImageType : uint8
{
    UI_Play_Score,
    UI_Play_FruitList,
    UI_Play_NextFruit
};

// 앵커 위치 열거형
UENUM(BlueprintType)
enum class EWidgetAnchor : uint8
{
    TopLeft,
    TopRight,
    MiddleLeft,
    MiddleRight,
    BottomLeft,
    BottomRight,
    Center
};

class UCanvasPanelSlot;
class UImage;
class UTexture2D;

UCLASS()
class UE_FRUITMOUNTAIN_API UUIHelper : public UObject
{
    GENERATED_BODY()
    
public:
    // 앵커 기반 슬롯 위치 설정
    static void SetAnchorForSlot(UCanvasPanelSlot* CanvasSlot, EWidgetAnchor Anchor, float PaddingX, float PaddingY);
    
    // 텍스처 로드 및 적용
    static UTexture2D* LoadAndApplyTexture(UImage* ImageWidget, const FString& TexturePath);
};