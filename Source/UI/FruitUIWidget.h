#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h" // CanvasPanel 헤더 추가
#include "FruitUIWidget.generated.h"

UENUM(BlueprintType)
enum class EWidgetPosition : uint8
{
    LeftTop     UMETA(DisplayName = "Left Top"),
    LeftMiddle  UMETA(DisplayName = "Left Middle"),
    RightTop    UMETA(DisplayName = "Right Top")
};

/**
 * 간단한 이미지 UI 위젯 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitUIWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    // 기본 생성자
    UFruitUIWidget(const FObjectInitializer& ObjectInitializer);
    
    // 이미지 설정 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetImage(UTexture2D* NewTexture);
    
    // 위치 설정 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetWidgetPosition(EWidgetPosition Position);
    
protected:
    virtual void NativeConstruct() override;
    
    // 위젯 위치 설정 구현
    void UpdateWidgetPosition();
    
private:
    // 이미지 컴포넌트
    UPROPERTY(meta = (BindWidget))
    UImage* IconImage = nullptr;
    
    // 위치 정보
    UPROPERTY()
    EWidgetPosition WidgetPosition;
    
    // 패딩 값 - 클래스 멤버로 선언하여 충돌 방지
    UPROPERTY()
    float EdgePadding = 50.0f;
};