#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIWidgetUtility.h"
#include "UIWidgetRenderer.generated.h"

UENUM(BlueprintType)
enum class EWidgetImageType : uint8
{
    UI_Play_Score,
    UI_Play_FruitList,
    UI_Play_NextFruit,
    UI_Title_Logo,
    UI_Title_Menu,
    UI_Title_Select,
    UI_Title_GameModeMenu,
    UI_Title_GameModeSelect,
    UI_None
};

class UCanvasPanel;
class UImage;

/**
 * 게임 내 텍스처를 표시하는 위젯
 * 싱글톤 패턴으로 구현되어 하나의 인스턴스만 유지
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UUIWidgetRenderer : public UUserWidget
{
    GENERATED_BODY()
    
public:
    // 위젯 인스턴스 생성 또는 기존 인스턴스 반환
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay", meta = (WorldContext = "WorldContextObject"))
    static UUIWidgetRenderer* CreateDisplayWidget(UObject* WorldContextObject);
    
    // 인스턴스가 유효한지 확인
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay")
    static bool IsInstanceValid();
    
    // Instance에 접근할 수 있는 getter 추가
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay")
    static UUIWidgetRenderer* GetInstance()
    {
        return Instance;
    }
    
    // 싱글톤 인스턴스 정리
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay")
    static void ClearInstance()
    {
        Instance = nullptr;
    }
    
    // 모든 기본 이미지 설정
    UFUNCTION(BlueprintCallable, Category = "UI TextureDisplay")
    void SetupPlayImages();

    // 이미지 위젯 생성
    UImage* PrepareUIWidget(
        EWidgetImageType ImageType,
        const FString& TexturePath,
        const FVector2D& CustomSize,
        float PaddingX,
        float PaddingY);

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
    void RenderUIImage(UImage*& ImageWidget, EWidgetAnchor Anchor, const FString& TexturePath, const FVector2D& CustomSize = FVector2D::ZeroVector, float PaddingX = 0.0f, float PaddingY = 0.0f);
    
    UImage* GetLogoImage() const { return UI_Title_Logo; }
    UImage* GetMenuImage() const { return UI_Title_Menu; }

protected:
    // 위젯 생성 시 호출됨
    virtual void NativeConstruct() override;
    
    // 위젯 소멸 시 호출됨
    virtual void NativeDestruct() override;
    
    // 싱글톤 인스턴스
    static UUIWidgetRenderer* Instance;
    
private:
    // 루트 캔버스 패널
    UPROPERTY()
    UCanvasPanel* Canvas;
    
    // 미리 정의된 UI 이미지 참조들
    UPROPERTY()
    UImage* UI_Play_Score;
    
    UPROPERTY()
    UImage* UI_Play_FruitList;
    
    UPROPERTY()
    UImage* UI_Play_NextFruit;

    // 멤버 변수도 추가
    UPROPERTY()
    UImage* UI_Title_Logo;

    UPROPERTY()
    UImage* UI_Title_Menu;

    UPROPERTY()
    UImage* UI_Title_Select;

    UPROPERTY()
    UImage* UI_Title_GameModeMenu;

    UPROPERTY()
    UImage* UI_Title_GameModeSelect;
};