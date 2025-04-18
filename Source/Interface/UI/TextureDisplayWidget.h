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
    
    // 소멸 시 호출
    virtual void NativeDestruct() override;
    
    // 이미지 설정
    void SetupAllImages();
    
    // 특정 위치에 이미지 설정 (블루프린트에서도 호출 가능)
    UFUNCTION(BlueprintCallable, Category="UI")
    void SetImageTexture(EWidgetImageType Position, const FString& TexturePath, const FVector2D& CustomSize = FVector2D(0, 0), float PaddingX = 20.0f, float PaddingY = 20.0f);
    
protected:
    // 이미지 컴포넌트
    UPROPERTY()
    UImage* UI_Play_Score;
    
    UPROPERTY()
    UImage* UI_Play_FruitList;
    
    UPROPERTY()
    UImage* UI_Play_NextFruit;
    
    // 캔버스 패널 참조
    UPROPERTY()
    UCanvasPanel* Canvas;
    
    // 앵커 기반 이미지 설정 - 크기 직접 지정 가능
    void SetupImageWithTexture(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath, const FVector2D& CustomSize = FVector2D(0, 0), float PaddingX = 20.0f, float PaddingY = 20.0f);

    // 헤더에 애셋 레퍼런스 변수 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSoftObjectPtr<UTexture2D> ScoreTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSoftObjectPtr<UTexture2D> FruitListTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSoftObjectPtr<UTexture2D> NextFruitTexture;

    // 로드 함수 추가
    UTexture2D* LoadTexture(TSoftObjectPtr<UTexture2D>& TexturePtr, const FString& FallbackPath);
};