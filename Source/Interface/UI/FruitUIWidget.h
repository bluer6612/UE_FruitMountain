#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Framework/UE_FruitMountainGameInstance.h"
#include "FruitUIWidget.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitUIWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    // 생성자 추가
    UFruitUIWidget(const FObjectInitializer& ObjectInitializer);
    
    // 위젯 초기화 함수
    virtual void NativeConstruct() override;
    
    // 위젯 소멸 함수
    virtual void NativeDestruct() override;
    
    // 위치 설정 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetWidgetPosition(EWidgetPosition NewPosition);
    
    // 이미지 설정 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetImage(UTexture2D* NewTexture);
    
    // 위치 업데이트 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateWidgetPosition();
    
protected:
    // 이미지 컴포넌트
    UPROPERTY(BlueprintReadOnly, Category = "UI")
    UImage* IconImage;
    
    // 위치 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EWidgetPosition WidgetPosition;
    
    // 패딩 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    float EdgePadding = 50.0f;
};