#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimpleTextureWidget.generated.h"

// 위젯 위치 열거형
UENUM(BlueprintType)
enum class EWidgetImageType : uint8
{
    LeftTop,
    LeftMiddle,
    RightTop
};

UCLASS()
class UE_FRUITMOUNTAIN_API USimpleTextureWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 정적 인스턴스
    static USimpleTextureWidget* Instance;
    
    // 생성 함수
    UFUNCTION(BlueprintCallable, Category="UI")
    static USimpleTextureWidget* CreateSimpleUI(UObject* WorldContextObject);
    
    // 초기화 시 호출
    virtual void NativeConstruct() override;
    
    // 이미지 설정
    void SetupAllImages();
    
    // 특정 위치에 이미지 설정 (블루프린트에서도 호출 가능)
    UFUNCTION(BlueprintCallable, Category="UI")
    void SetImageTexture(EWidgetImageType Position, const FString& TexturePath);
    
protected:
    // 이미지 컴포넌트
    UPROPERTY()
    class UImage* LeftTopImage;
    
    UPROPERTY()
    class UImage* LeftMiddleImage;
    
    UPROPERTY()
    class UImage* RightTopImage;
    
    // 헬퍼 함수
    UImage* GetImageForType(EWidgetImageType Type);
    void SetupImageWithTexture(UImage* ImageWidget, const FVector2D& Position, const FString& TexturePath);
};