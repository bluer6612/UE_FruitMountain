#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "ScoreWidgetAnimator.generated.h"

// 전방 선언
class UScoreDisplayWidget;

// 텍스트 블록 위치 정보
struct FTextBlockPositions
{
    UCanvasPanelSlot* ScoreSlot;
    UCanvasPanelSlot* ComboSlot;
    FVector2D ScoreInitialPos;
    FVector2D ComboInitialPos;
};

// 애니메이션 파라미터
struct FAnimationParameters
{
    float FadeDuration;
    float FadeInterval;
    int32 FadeSteps;
    float FadeStep;
    float TotalMoveDistance;
    float MoveStep;
};

UCLASS()
class UE_FRUITMOUNTAIN_API UScoreWidgetAnimator : public UObject
{
    GENERATED_BODY()
    
public:
    UScoreWidgetAnimator();
    
    // 텍스트 블록을 애니메이션하기 위해 설정
    void SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText);
    
    // 페이드 아웃 애니메이션 시작
    void StartFadeOutAnimation(UObject* WorldContextObject, float Delay = 2.0f);
    
    // 애니메이션 취소
    void CancelAnimation();
    
    // 텍스트 블록 속성 초기화
    void ResetTextBlockProperties();
    
    // 스코어 값 재설정
    void ResetScoreValues();
    
private:
    // 참조용 텍스트 블록
    UPROPERTY()
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY()
    UTextBlock* ComboMultiplierTextBlock;
    
    // 타이머 핸들
    FTimerHandle AnimTimerHandle;
    
    // 애니메이션 상태 변수
    int32 TotalScoreGain;
    int32 CurrentScoreGain;
    float CurrentComboMultiplier;
    bool bScoreTextActive;
    bool bAnimationActive;
    
    // 헬퍼 함수들
    bool AreTextBlocksValid() const;
    FTextBlockPositions GetTextBlockPositions() const;
    FAnimationParameters SetupAnimationParameters() const;
    FTimerDelegate CreateFadeDelegate(const FTextBlockPositions& Positions, const FAnimationParameters& Params);
    void ExecuteFadeOut();
    
    // 애니메이션 종료 처리 함수 (추가됨)
    void ExecuteAnimationEnd();
};