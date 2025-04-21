#include "ScoreWidgetAnimator.h"
#include "Components/CanvasPanelSlot.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ScoreDisplayWidget.h"

UScoreWidgetAnimator::UScoreWidgetAnimator()
{
    ScoreTextBlock = nullptr;
    ComboMultiplierTextBlock = nullptr;
    TotalScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
    bScoreTextActive = false;
    bAnimationActive = false;
}

void UScoreWidgetAnimator::SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText)
{
    ScoreTextBlock = InScoreText;
    ComboMultiplierTextBlock = InComboText;
}

void UScoreWidgetAnimator::StartFadeOutAnimation(UObject* WorldContextObject, float Delay)
{
    // 유효성 검사를 인라인으로 수행 (AreTextBlocksValid 함수 대체)
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        return;
    }

    // 기존 애니메이션 취소
    CancelAnimation();
    
    // 위치 정보 직접 가져오기 (GetTextBlockPositions 함수 대체)
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
        FTimerDelegate TimerDelegate = CreateFadeDelegate(ScorePos, ComboPos);
        World->GetTimerManager().SetTimer(AnimTimerHandle, TimerDelegate, Delay, false);
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
    
    // 콤보 배율 초기화
    CurrentComboMultiplier = 1.0f;
}

void UScoreWidgetAnimator::ExecuteFadeOut()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        return;
    }
    
    UWorld* World = ScoreTextBlock->GetWorld();
    if (!World) return;
    
    // 위치 및 애니메이션 설정
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
    World->GetTimerManager().SetTimer(
        AnimTimerHandle, 
        FadeDelegate, 
        0.025f, 
        true
    );
}

FTimerDelegate UScoreWidgetAnimator::CreateFadeDelegate(const FVector2D& ScorePos, const FVector2D& ComboPos)
{
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, ScorePos, ComboPos]() mutable
    {
        static int32 CurrentStep = 0;
        CurrentStep++;
        
        // 애니메이션 값 계산
        float Alpha = 1.0f - (CurrentStep * 0.05f);
        float CurrentMoveX = CurrentStep * -2.0f;
        
        // 스코어 텍스트 업데이트
        if (ScoreTextBlock)
        {
            // 투명도 조절
            FLinearColor TextColor = ScoreTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            TextColor.A = Alpha;
            ScoreTextBlock->SetColorAndOpacity(TextColor);
            
            // 위치 이동
            FVector2D NewPos = ScorePos;
            NewPos.X += CurrentMoveX;
            if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
            {
                ScoreSlot->SetPosition(NewPos);
            }
        }
        
        // 콤보 텍스트 업데이트
        if (ComboMultiplierTextBlock)
        {
            // 투명도 조절
            FLinearColor ComboColor = ComboMultiplierTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
            
            // 위치 이동
            FVector2D NewPos = ComboPos;
            NewPos.X += CurrentMoveX;
            if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
            {
                ComboSlot->SetPosition(NewPos);
            }
        }
        
        // 애니메이션 종료
        if (CurrentStep >= 20)
        {
            CurrentStep = 0;
            UWorld* World = ScoreTextBlock->GetWorld();
            if (World)
            {
                World->GetTimerManager().ClearTimer(AnimTimerHandle);
            }
            
            // 애니메이션 종료 함수 호출
            ExecuteAnimationEnd();
        }
    });
    
    return FadeDelegate;
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
    
    // 점수 초기화
    CurrentScoreGain = 0;
    
    bAnimationActive = false;
}