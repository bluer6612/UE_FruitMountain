#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestUIWidget.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UTestUIWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    // 생성자 추가
    UTestUIWidget(const FObjectInitializer& ObjectInitializer);
    
    // 위젯 사전 초기화 함수 추가
    virtual void NativePreConstruct() override;
    
    // 위젯 초기화 함수
    virtual void NativeConstruct() override;
    
    // 빨간색 블록 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowRedBlock();

    // 위젯 소멸 함수 추가
    virtual void NativeDestruct() override;
    
protected:
    // 이미지 컴포넌트
    UPROPERTY()
    class UImage* TestImage;
};