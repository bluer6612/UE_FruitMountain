#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "ScoreWidgetAnimator.generated.h"

// 전방 선언
class UScoreDisplayWidget;

// 애니메이션 파라미터
struct FScoreAnimParams
{
    float FrameInterval = 0.02f;    // 프레임 간 시간 간격
    float AlphaStepSize = 0.05f;    // 투명도 감소량/프레임
    float MoveStepSize = -2.0f;     // 이동 거리/프레임
    int32 TotalSteps = 20;          // 총 애니메이션 스텝 수
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
    void StartFadeOutAnimation(UObject* WorldContextObject, float Delay = 1.0f);
    
    // 애니메이션 취소 및 속성 초기화
    void CancelAnimation();
    
private:
    // 참조용 텍스트 블록
    UPROPERTY()
    UTextBlock* ScoreTextBlock;
    
    UPROPERTY()
    UTextBlock* ComboMultiplierTextBlock;
    
    // 타이머 핸들
    FTimerHandle AnimTimerHandle;
    
    // 애니메이션 상태 관리
    float CurrentComboMultiplier;
    bool bAnimationActive;
    int32 CurrentAnimStep;
    
    // 헬퍼 함수들 - 일관된 파라미터 사용
    FScoreAnimParams SetupAnimationParameters() const;
    FTimerDelegate CreateFadeDelegate(const FVector2D& ScorePos, const FVector2D& ComboPos);
    void ExecuteFadeOut();
    void ExecuteAnimationEnd();
    
    // 텍스트 블록 속성 초기화
    void ResetTextBlockProperties();
};