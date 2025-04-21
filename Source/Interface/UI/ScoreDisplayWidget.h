#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ScoreDisplayWidget.generated.h"

// 전방 선언
class UScoreWidgetAnimator;

UCLASS()
class UE_FRUITMOUNTAIN_API UScoreDisplayWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UScoreDisplayWidget(const FObjectInitializer& ObjectInitializer);

    // 정적 인스턴스 - 싱글톤 패턴
    static UScoreDisplayWidget* Instance;
    
    // 위젯 클래스 참조 (블루프린트 위젯 클래스)
    static TSubclassOf<UUserWidget> ScoreWidgetClass;
    
    // 위젯 생성 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score", meta = (WorldContext = "WorldContextObject"))
    static UScoreDisplayWidget* CreateScoreWidget(UObject* WorldContextObject);
    
    // 점수 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void DisplayScoreGain(int32 Score, int32 ComboCount, float ComboMultiplier);

    static const FVector2D SCORE_TEXT_POS;
    static const FVector2D COMBO_TEXT_POS;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ComboMultiplierTextBlock;
    
    UPROPERTY()
    UScoreWidgetAnimator* WidgetAnimator;
    
    // 그림자 이미지 참조
    UPROPERTY()
    UImage* ScoreTextShadow;
    
    UPROPERTY()
    UImage* ComboTextShadow;
    
    // 점수 데이터
    int32 PendingScoreGain;
    float CurrentComboMultiplier;
    bool bScoreTextActive;
    
    // 타이머 핸들
    FTimerHandle ScoreAnimTimerHandle;
    
private:
    // 헬퍼 함수들
    void InitializeTextBlocks();
    void SetupTextBlock(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, FVector2D Pos);
    void AddHorizontalShadow(UTextBlock* TextBlock);
    
    // 유효성 검사 및 초기화 헬퍼
    static bool IsInstanceValid();
    static APlayerController* GetValidPlayerController(UObject* WorldContextObject);
    static bool LoadWidgetClassIfNeeded();
};