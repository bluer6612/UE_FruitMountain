#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ScoreWidgetAnimator.h"
#include "ScoreDisplayWidget.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API UScoreDisplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // 위치 및 색상 상수
    static const FVector2D SCORE_TEXT_POS;
    static const FVector2D COMBO_TEXT_POS;
    static const FLinearColor SCORE_YELLOW_COLOR;

    // 생성자
    UScoreDisplayWidget(const FObjectInitializer& ObjectInitializer);
    
    // 위젯 라이프사이클 함수
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void BeginDestroy() override;
    
    // 위젯 생성 및 접근 인터페이스
    static UScoreDisplayWidget* CreateScoreWidget(UObject* WorldContextObject);
    static UScoreDisplayWidget* GetInstance() { return Instance; }
    
    // 점수 표시 함수
    void DisplayScoreGain(int32 Score, int32 ComboCount, float ComboMultiplier);
    void ResetComboDisplay();
    
    // 애니메이터 접근자
    UScoreWidgetAnimator* GetWidgetAnimator() const { return WidgetAnimator; }

    // 인스턴스 유효성 검사
    static bool IsInstanceValid();

    // 인스턴스 제거
    static void ClearInstance();
    
protected:
    // 텍스트 블록 초기화
    void InitializeTextBlocks();
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* ComboMultiplierTextBlock;

private:
    // 애니메이터
    UPROPERTY()
    UScoreWidgetAnimator* WidgetAnimator;
    
    // 점수 관련 변수
    int32 TotalScoreGain;
    int32 CurrentScoreGain;
    float CurrentComboMultiplier;
    bool bScoreTextActive;
    
    // 정적 인스턴스 및 클래스 참조
    static UScoreDisplayWidget* Instance;
    static TSubclassOf<UUserWidget> ScoreWidgetClass;
};