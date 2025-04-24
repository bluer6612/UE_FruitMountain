#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIHelper.h"
#include "TextureDisplayWidget.generated.h"

class UCanvasPanel;
class UImage;

/**
 * 게임 내 텍스처를 표시하는 위젯
 * 싱글톤 패턴으로 구현되어 하나의 인스턴스만 유지
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UTextureDisplayWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    /** 표준 UI 이미지 타입 */
    UENUM(BlueprintType)
    enum class EWidgetImageType : uint8
    {
        UI_Play_Score,     // 왼쪽 상단 점수판
        UI_Play_FruitList, // 왼쪽 하단 과일 목록
        UI_Play_NextFruit, // 오른쪽 상단 다음 과일
    };
    
    /** 위젯 인스턴스 생성 또는 기존 인스턴스 반환 */
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay", meta = (WorldContext = "WorldContextObject"))
    static UTextureDisplayWidget* CreateDisplayWidget(UObject* WorldContextObject);
    
    /** 인스턴스가 유효한지 확인 */
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay")
    static bool IsInstanceValid();
    
    /** 모든 기본 이미지 설정 */
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay")
    void SetupAllImages();
    
    // === 함수 오버로드 정리 ===
    
    /**
     * 미리 정의된 UI 요소에 이미지 설정
     * @param ImageType - 이미지 위젯 타입 (UI_Play_Score, UI_Play_FruitList 등)
     * @param TexturePath - 텍스처 경로
     * @param CustomSize - 이미지 크기 (0,0이면 원본 크기 사용)
     * @param PaddingX - X축 패딩
     * @param PaddingY - Y축 패딩
     */
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay")
    void SetupImage(EWidgetImageType ImageType, const FString& TexturePath, 
                    const FVector2D& CustomSize = FVector2D::ZeroVector, 
                    float PaddingX = 0.0f, float PaddingY = 0.0f);
    
    /**
     * 직접 제공된 이미지 위젯에 텍스처 설정
     * 동적 UI 요소나 커스텀 배치가 필요한 상황에서 사용
     * @param ImageWidget - 설정할 이미지 위젯 참조
     * @param Anchor - 화면상 앵커 위치
     * @param TexturePath - 텍스처 경로
     * @param CustomSize - 이미지 크기 (0,0이면 원본 크기 사용)
     * @param PaddingX - X축 패딩
     * @param PaddingY - Y축 패딩
     */
    void SetupImage(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath,
                    const FVector2D& CustomSize = FVector2D::ZeroVector, 
                    float PaddingX = 0.0f, float PaddingY = 0.0f);
    
protected:
    /** 위젯 생성 시 호출됨 */
    virtual void NativeConstruct() override;
    
    /** 위젯 소멸 시 호출됨 */
    virtual void NativeDestruct() override;
    
private:
    /** 싱글톤 인스턴스 */
    static UTextureDisplayWidget* Instance;
    
    /** 루트 캔버스 패널 */
    UPROPERTY()
    UCanvasPanel* Canvas;
    
    // 미리 정의된 UI 이미지 참조들
    UPROPERTY()
    UImage* UI_Play_Score;
    
    UPROPERTY()
    UImage* UI_Play_FruitList;
    
    UPROPERTY()
    UImage* UI_Play_NextFruit;
};