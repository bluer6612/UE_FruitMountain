#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/TextBlock.h"
#include "ScoreWidgetAnimator.generated.h"

// 단순 델리게이트
DECLARE_DELEGATE(FOnAnimationComplete);

// 애니메이션 파라미터 구조체
USTRUCT()
struct FScoreAnimParams
{
    GENERATED_BODY()
    
    int32 TotalSteps = 20;
    float FrameInterval = 0.02f;
    float AlphaStepSize = 0.05f;
    float MoveStepSize = 5.f;
};

UCLASS()
class UE_FRUITMOUNTAIN_API UScoreWidgetAnimator : public UObject
{
    GENERATED_BODY()
    
public:
    UScoreWidgetAnimator();
    virtual void BeginDestroy() override;
    
    // 애니메이터 초기화 함수
    void Initialize(UTextBlock* InScoreTextBlock_C, UTextBlock* InComboTextBlock);
    
    // 주요 인터페이스 함수
    void SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText);
    void FadeOutBoth(float Delay = 0.0f);
    void CancelAnimation();
    
    // 점수 텍스트 애니메이션
    void AnimateScoreText(int32 Score, float Delay = 2.0f);
    
    // 콤보 텍스트 애니메이션
    void AnimateComboText(int32 ComboCount, float ComboMultiplier, float Delay = 2.0f);
    
    // 콜백
    FOnAnimationComplete OnAnimationComplete;
    
    void StartFadeOutAnimation(UObject* WorldContextObject, float Delay);
    
protected:
    // 텍스트 블록
    UPROPERTY()
    UTextBlock* ScoreTextBlock_C;
    
    UPROPERTY()
    UTextBlock* ComboMultiplierTextBlock_C;
    
    // 타이머 핸들
    FTimerHandle DelayTimerHandle;
    FTimerHandle AnimTimerHandle;
    
    // 애니메이션 상태 추적
    UPROPERTY()
    bool bAnimationActive;
    
    int32 CurrentAnimStep;
    
    // 내부 구현 함수
    void ExecuteFadeOut();
    FTimerDelegate CreateFadeDelegate(const FVector2D& ScorePos, const FVector2D& ComboPos);
    void ExecuteAnimationEnd();
};