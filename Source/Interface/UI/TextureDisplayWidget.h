#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIHelper.h"
#include "TextureDisplayWidget.generated.h"

class UImage;
class UCanvasPanel;
class UCanvasPanelSlot;

UCLASS()
class UE_FRUITMOUNTAIN_API UTextureDisplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 정적 인스턴스
    static UTextureDisplayWidget* Instance;
    
    // 생성 함수
    UFUNCTION(BlueprintCallable, Category="UI")
    static UTextureDisplayWidget* CreateDisplayWidget(UObject* WorldContextObject);
    
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
    UImage* LeftTopImage;
    
    UPROPERTY()
    UImage* LeftMiddleImage;
    
    UPROPERTY()
    UImage* RightTopImage;
    
    // 캔버스 패널 참조
    UPROPERTY()
    UCanvasPanel* Canvas;
    
    // 이미지 설정 함수 (UI 헬퍼 클래스로 기능 분리)
    void SetupImageWithTexture(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath);
};