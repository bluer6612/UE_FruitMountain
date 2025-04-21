#include "ScoreWidgetAnimator.h"
#include "Components/CanvasPanelSlot.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ScoreDisplayWidget.h"

UScoreWidgetAnimator::UScoreWidgetAnimator()
{
    ScoreTextBlock = nullptr;
    ComboMultiplierTextBlock = nullptr;
    CurrentComboMultiplier = 1.0f;
    bAnimationActive = false;
    CurrentAnimStep = 0;
}

// 애니메이션 파라미터 설정 함수
FScoreAnimParams UScoreWidgetAnimator::SetupAnimationParameters() const
{
    // 기본값으로 초기화된 구조체 반환
    return FScoreAnimParams();
}

void UScoreWidgetAnimator::SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText)
{
    ScoreTextBlock = InScoreText;
    ComboMultiplierTextBlock = InComboText;
}

void UScoreWidgetAnimator::StartFadeOutAnimation(UObject* WorldContextObject, float Delay)
{
    // 유효성 검사를 인라인으로 수행
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        return;
    }

    // 기존 애니메이션 취소
    CancelAnimation();
    
    // 위치 정보 직접 가져오기
    FVector2D ScorePos = UScoreDisplayWidget::SCORE_TEXT_POS;
    FVector2D ComboPos = UScoreDisplayWidget::COMBO_TEXT_POS;
    
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        ScorePos = ScoreSlot->GetPosition();
    }
    
    if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
    {
        ComboPos = ComboSlot->GetPosition();
    }
    
    // 타이머 설정 및 애니메이션 시작
    UWorld* World = WorldContextObject->GetWorld();
    if (World)
    {
        // 타이머 함수: 지연 후 ExecuteFadeOut 함수를 직접 호출
        FTimerDelegate DelayedStart;
        DelayedStart.BindUObject(this, &UScoreWidgetAnimator::ExecuteFadeOut);
        
        World->GetTimerManager().SetTimer(AnimTimerHandle, DelayedStart, Delay, false);
        bAnimationActive = true;
    }
}

void UScoreWidgetAnimator::CancelAnimation()
{
    if (!bAnimationActive || !ScoreTextBlock || !ComboMultiplierTextBlock)
        return;
    
    UWorld* World = ScoreTextBlock->GetWorld();
    if (!World) return;
    
    if (World->GetTimerManager().IsTimerActive(AnimTimerHandle))
    {
        World->GetTimerManager().ClearTimer(AnimTimerHandle);
    }
    
    // 타이머 취소 시 텍스트 블록 속성 초기화
    ResetTextBlockProperties();
    
    bAnimationActive = false;
}

void UScoreWidgetAnimator::ResetTextBlockProperties()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        return;
    }

    // 색상 및 투명도 초기화
    ScoreTextBlock->SetColorAndOpacity(UScoreDisplayWidget::BRIGHT_YELLOW_COLOR);
    ComboMultiplierTextBlock->SetColorAndOpacity(UScoreDisplayWidget::BRIGHT_YELLOW_COLOR);
    
    // 위치 초기화
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        ScoreSlot->SetPosition(UScoreDisplayWidget::SCORE_TEXT_POS);
    }
    
    if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
    {
        ComboSlot->SetPosition(UScoreDisplayWidget::COMBO_TEXT_POS);
    }
    
    // 콤보 배율 및 애니메이션 상태 초기화
    CurrentComboMultiplier = 1.0f;
    CurrentAnimStep = 0;
}

FTimerDelegate UScoreWidgetAnimator::CreateFadeDelegate(const FVector2D& ScorePos, const FVector2D& ComboPos)
{
    // 애니메이션 파라미터 가져오기
    FScoreAnimParams Params = SetupAnimationParameters();
    
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, ScorePos, ComboPos, Params]() mutable
    {
        // static 변수 대신 멤버 변수 사용
        CurrentAnimStep++;
        
        // 매개변수화된 상수 사용하여 애니메이션 값 계산
        float Alpha = 1.0f - (CurrentAnimStep * Params.AlphaStepSize);
        float CurrentMoveX = CurrentAnimStep * Params.MoveStepSize;
        
        // 텍스트 블록 색상 및 위치 업데이트
        if (ScoreTextBlock)
        {
            FLinearColor ScoreColor = UScoreDisplayWidget::BRIGHT_YELLOW_COLOR;
            ScoreColor.A = Alpha;
            ScoreTextBlock->SetColorAndOpacity(ScoreColor);
            
            if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
            {
                ScoreSlot->SetPosition(FVector2D(ScorePos.X + CurrentMoveX, ScorePos.Y));
            }
        }
        
        if (ComboMultiplierTextBlock && ComboMultiplierTextBlock->GetVisibility() == ESlateVisibility::HitTestInvisible)
        {
            FLinearColor ComboColor = UScoreDisplayWidget::BRIGHT_YELLOW_COLOR;
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
            
            if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
            {
                ComboSlot->SetPosition(FVector2D(ComboPos.X + CurrentMoveX, ComboPos.Y));
            }
        }
        
        // 애니메이션 종료 조건
        if (CurrentAnimStep >= Params.TotalSteps)
        {
            UWorld* World = ScoreTextBlock->GetWorld();
            if (World)
            {
                World->GetTimerManager().ClearTimer(AnimTimerHandle);
            }
            
            // 애니메이션 종료 처리
            ExecuteAnimationEnd();
            
            // 멤버 변수 초기화
            CurrentAnimStep = 0;
        }
    });
    
    return FadeDelegate;
}

void UScoreWidgetAnimator::ExecuteFadeOut()
{
    if (!ScoreTextBlock || !ScoreTextBlock->GetWorld())
    {
        return;
    }
    
    // 애니메이션 파라미터 사용
    FScoreAnimParams Params = SetupAnimationParameters();
    
    // 현재 위치 가져오기
    FVector2D ScorePos = UScoreDisplayWidget::SCORE_TEXT_POS;
    FVector2D ComboPos = UScoreDisplayWidget::COMBO_TEXT_POS;
    
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        ScorePos = ScoreSlot->GetPosition();
    }
    
    if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
    {
        ComboPos = ComboSlot->GetPosition();
    }
    
    // 애니메이션 타이머 설정
    FTimerDelegate FadeDelegate = CreateFadeDelegate(ScorePos, ComboPos);
    UWorld* World = ScoreTextBlock->GetWorld();
    
    if (World)
    {
        World->GetTimerManager().SetTimer(
            AnimTimerHandle, 
            FadeDelegate, 
            Params.FrameInterval,  // 매개변수화된 프레임 간격 사용
            true
        );
    }
}

void UScoreWidgetAnimator::ExecuteAnimationEnd()
{
    // 텍스트 숨기기
    if (ScoreTextBlock)
    {
        ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 콤보 숨기기
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    bAnimationActive = false;
}