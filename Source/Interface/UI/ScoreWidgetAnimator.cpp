#include "ScoreWidgetAnimator.h"
#include "Components/CanvasPanelSlot.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ScoreDisplayWidget.h"

UScoreWidgetAnimator::UScoreWidgetAnimator()
{
    ScoreTextBlock = nullptr;
    ComboMultiplierTextBlock = nullptr;
    bAnimationActive = false;
    CurrentAnimStep = 0;
}

void UScoreWidgetAnimator::BeginDestroy()
{
    CancelAnimation();
    
    ScoreTextBlock = nullptr;
    ComboMultiplierTextBlock = nullptr;
    
    Super::BeginDestroy();
}

void UScoreWidgetAnimator::SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText)
{
    ScoreTextBlock = InScoreText;
    ComboMultiplierTextBlock = InComboText;
}

void UScoreWidgetAnimator::StartFadeOutAnimation(UObject* WorldContextObject, float Delay)
{
    // 유효성 검사
    if (!ScoreTextBlock || !ScoreTextBlock->GetWorld())
    {
        return;
    }
    
    // 기존 애니메이션 취소
    CancelAnimation();
    
    // 타이머 설정
    UWorld* World = WorldContextObject->GetWorld();
    if (World)
    {
        FTimerDelegate DelayedStart;
        DelayedStart.BindUObject(this, &UScoreWidgetAnimator::ExecuteFadeOut);
        World->GetTimerManager().SetTimer(DelayTimerHandle, DelayedStart, Delay, false);
        bAnimationActive = true;
    }
}

void UScoreWidgetAnimator::CancelAnimation()
{
    if (ScoreTextBlock && ScoreTextBlock->GetWorld())
    {
        UWorld* World = ScoreTextBlock->GetWorld();
        World->GetTimerManager().ClearTimer(DelayTimerHandle);
        World->GetTimerManager().ClearTimer(AnimTimerHandle);
        
        // 위치 복원
        if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
        {
            ScoreSlot->SetPosition(UScoreDisplayWidget::SCORE_TEXT_POS);
        }
        
        if (ComboMultiplierTextBlock)
        {
            if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
            {
                ComboSlot->SetPosition(UScoreDisplayWidget::COMBO_TEXT_POS);
            }
        }
    }
    
    bAnimationActive = false;
    CurrentAnimStep = 0;
}

FTimerDelegate UScoreWidgetAnimator::CreateFadeDelegate(const FVector2D& ScorePos, const FVector2D& ComboPos)
{
    FScoreAnimParams Params;
    
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, ScorePos, ComboPos, Params]() mutable
    {
        if (!ScoreTextBlock || !ScoreTextBlock->GetWorld())
        {
            CancelAnimation();
            return;
        }
        
        CurrentAnimStep++;
        
        // 페이드아웃 계산
        float Alpha = FMath::Max(1.0f - (CurrentAnimStep * Params.AlphaStepSize), 0.0f);
        float CurrentMoveX = CurrentAnimStep * Params.MoveStepSize;
        
        // ScoreTextBlock 애니메이션
        FSlateColor CurrentScoreColor = ScoreTextBlock->GetColorAndOpacity();
        FLinearColor ScoreColor = CurrentScoreColor.GetSpecifiedColor();
        ScoreColor.A = Alpha;
        ScoreTextBlock->SetColorAndOpacity(FSlateColor(ScoreColor));
        
        if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
        {
            ScoreSlot->SetPosition(FVector2D(ScorePos.X - CurrentMoveX, ScorePos.Y));
        }
        
        // ComboMultiplierTextBlock 애니메이션
        if (ComboMultiplierTextBlock)
        {
            FSlateColor CurrentComboColor = ComboMultiplierTextBlock->GetColorAndOpacity();
            FLinearColor ComboColor = CurrentComboColor.GetSpecifiedColor();
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(FSlateColor(ComboColor));
            
            if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
            {
                ComboSlot->SetPosition(FVector2D(ComboPos.X - CurrentMoveX, ComboPos.Y));
            }
        }
        
        // 애니메이션 종료 체크
        if (CurrentAnimStep >= Params.TotalSteps)
        {
            // 가시성으로 즉시 숨김
            ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
            if (ComboMultiplierTextBlock)
            {
                ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
            }
            
            // 타이머 중지
            ScoreTextBlock->GetWorld()->GetTimerManager().ClearTimer(AnimTimerHandle);
            
            // 애니메이션 종료 처리
            ExecuteAnimationEnd();
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
    
    // 텍스트 블록 가시성 설정
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
        
    // 항상 정적 위치 상수 사용
    FVector2D ScorePos = UScoreDisplayWidget::SCORE_TEXT_POS;
    FVector2D ComboPos = UScoreDisplayWidget::COMBO_TEXT_POS;
    
    // 위치 강제 설정
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        ScoreSlot->SetPosition(ScorePos);
    }
    
    if (ComboMultiplierTextBlock && Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
    {
        Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot)->SetPosition(ComboPos);
    }
    
    // 애니메이션 파라미터 설정
    CurrentAnimStep = 0;
    
    // 타이머 설정
    UWorld* World = ScoreTextBlock->GetWorld();
    FTimerDelegate FadeDelegate = CreateFadeDelegate(ScorePos, ComboPos);
    
    World->GetTimerManager().SetTimer(
        AnimTimerHandle, 
        FadeDelegate, 
        FScoreAnimParams().FrameInterval, 
        true
    );
}

void UScoreWidgetAnimator::AnimateScoreText(int32 Score, float Delay)
{
    if (!ScoreTextBlock)
    {
        return;
    }
    
    // 항상 기본 정적 위치로 리셋
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        ScoreSlot->SetPosition(UScoreDisplayWidget::SCORE_TEXT_POS);
    }
    
    // 텍스트 설정
    FString ScoreText = FString::Printf(TEXT("+%d"), Score);
    ScoreTextBlock->SetText(FText::FromString(ScoreText));
    ScoreTextBlock->SetColorAndOpacity(UScoreDisplayWidget::BRIGHT_YELLOW_COLOR);
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UScoreWidgetAnimator::AnimateComboText(int32 ComboCount, float ComboMultiplier, float Delay)
{
    if (!ComboMultiplierTextBlock)
    {
        return;
    }
    
    // 항상 기본 정적 위치로 리셋
    if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
    {
        ComboSlot->SetPosition(UScoreDisplayWidget::COMBO_TEXT_POS);
    }
    
    // 콤보가 없으면 표시하지 않음
    if (ComboCount < 2)
    {
        ComboMultiplierTextBlock->SetText(FText::GetEmpty());
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    
    // 텍스트 설정
    FString ComboText = FString::Printf(TEXT("X%.1f"), ComboMultiplier);
    ComboMultiplierTextBlock->SetText(FText::FromString(ComboText));
    ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    ComboMultiplierTextBlock->SetColorAndOpacity(UScoreDisplayWidget::BRIGHT_YELLOW_COLOR);
}

void UScoreWidgetAnimator::FadeOutBoth(float Delay)
{
    if (ScoreTextBlock && ScoreTextBlock->GetWorld())
    {
        StartFadeOutAnimation(ScoreTextBlock->GetWorld(), Delay);
    }
}

void UScoreWidgetAnimator::ExecuteAnimationEnd()
{
    // 두 텍스트 모두 즉시 숨김
    if (ScoreTextBlock)
    {
        ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 애니메이션 종료 처리
    bAnimationActive = false;
    
    // 콜백 호출
    if (OnAnimationComplete.IsBound())
    {
        OnAnimationComplete.Execute();
    }
}