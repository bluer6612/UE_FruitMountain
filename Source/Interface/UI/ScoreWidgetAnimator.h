#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/TextBlock.h"
#include "ScoreWidgetAnimator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScoreAnimationEndDelegate);

// 애니메이션 구성을 위한 파라미터 구조체
USTRUCT()
struct FScoreAnimParams
{
    GENERATED_BODY()
    
    // 애니메이션 총 단계 수
    int32 TotalSteps = 25;
    // 프레임 간 간격 (초)
    float FrameInterval = 0.05f;
    // 알파 감소량 (단계당)
    float AlphaStepSize = 0.04f;
    // 이동 거리 (단계당)
    float MoveStepSize = 2.0f;
};

UCLASS()
class UE_FRUITMOUNTAIN_API UScoreWidgetAnimator : public UObject
{
    GENERATED_BODY()
    
public:
    UScoreWidgetAnimator();
    virtual void BeginDestroy() override;
    
    // 애니메이션에 사용될 텍스트 블록 설정
    void SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText);
    
    // 애니메이션 시작 함수
    void StartFadeOutAnimation(UObject* WorldContextObject, float Delay = 0.0f);
    
    // 애니메이션 취소 함수
    void CancelAnimation();
    
    // 애니메이션 완료 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Score Animation")
    FScoreAnimationEndDelegate OnAnimationEnd;
    
protected:
    // 텍스트 블록
    UPROPERTY()
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY()
    UTextBlock* ComboMultiplierTextBlock;
    
    // 애니메이션 타이머
    FTimerHandle AnimTimerHandle;
    
    // 애니메이션 상태 변수
    bool bAnimationActive;
    int32 CurrentAnimStep;
    float CurrentComboMultiplier;
    
    // 애니메이션 파라미터 설정
    FScoreAnimParams SetupAnimationParameters() const;
    
    // 페이드 아웃 실행 함수
    void ExecuteFadeOut();
    
    // 페이드 델리게이트 생성 함수
    FTimerDelegate CreateFadeDelegate(const FVector2D& ScorePos, const FVector2D& ComboPos);
    
    // 애니메이션 종료 처리 함수
    void ExecuteAnimationEnd();
    
    // 텍스트 블록 속성 초기화
    void ResetTextBlockProperties();
};