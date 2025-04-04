// SimpleTextureWidget.h - 단순한 자체 완성형 위젯
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimpleTextureWidget.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API USimpleTextureWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 정적 인스턴스 - 전역 접근용
    static USimpleTextureWidget* Instance;
    
    // 정적 생성 함수 - 참조 보장
    UFUNCTION(BlueprintCallable, Category="UI")
    static USimpleTextureWidget* CreateSimpleUI(UObject* WorldContextObject);
    
    // NativeConstruct 오버라이드
    virtual void NativeConstruct() override;
    
    // 세 위치에 이미지 표시
    UFUNCTION(BlueprintCallable, Category="UI")
    void SetupAllImages();
    
protected:
    // UI 구성요소
    UPROPERTY()
    class UImage* LeftTopImage;
    
    UPROPERTY()
    class UImage* LeftMiddleImage;
    
    UPROPERTY()
    class UImage* RightTopImage;
    
    // 내부 헬퍼 - 이미지 설정
    void SetupImage(UImage*& ImagePtr, const FVector2D& Position, const FString& TextureName);
};