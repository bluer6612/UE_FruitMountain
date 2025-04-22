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
    
    // 위젯 클래스 참조
    static TSubclassOf<UUserWidget> ScoreWidgetClass;
    
    // 위치 색상 상수
    static const FVector2D SCORE_TEXT_POS;
    static const FVector2D COMBO_TEXT_POS;
    static const FLinearColor BRIGHT_YELLOW_COLOR;
    
    // 위젯 생성 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score", meta = (WorldContext = "WorldContextObject"))
    static UScoreDisplayWidget* CreateScoreWidget(UObject* WorldContextObject);
    
    // 점수 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void DisplayScoreGain(int32 Score, int32 ComboCount, float ComboMultiplier);

    // 콤보 타이머가 끊어질 때 호출하는 함수
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    void ResetComboDisplay();

    // 인스턴스 유효성 검증
    UFUNCTION(BlueprintCallable, Category = "UI Score")
    static bool IsInstanceValid();
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ComboMultiplierTextBlock;
    
    UPROPERTY()
    UScoreWidgetAnimator* WidgetAnimator;
    
    // 점수 데이터
    int32 TotalScoreGain;  // 총 누적 점수 (애니메이션당)
    int32 CurrentScoreGain;  // 현재 표시되는 점수 (애니메이션당)
    float CurrentComboMultiplier;
    bool bScoreTextActive;
    
private:
    // 헬퍼 함수
    void InitializeTextBlocks();
    void SetupTextBlock(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, FVector2D Pos);
};