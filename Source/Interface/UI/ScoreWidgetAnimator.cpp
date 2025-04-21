#include "ScoreWidgetAnimator.h"
#include "Components/CanvasPanelSlot.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ScoreDisplayWidget.h"

UScoreWidgetAnimator::UScoreWidgetAnimator()
{
    ScoreTextBlock = nullptr;
    ComboMultiplierTextBlock = nullptr;
    PendingScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
    bScoreTextActive = false;
    bAnimationActive = false;
}

void UScoreWidgetAnimator::SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText)
{
    ScoreTextBlock = InScoreText;
    ComboMultiplierTextBlock = InComboText;
}

bool UScoreWidgetAnimator::AreTextBlocksValid() const
{
    return ScoreTextBlock && ComboMultiplierTextBlock;
}

FTextBlockPositions UScoreWidgetAnimator::GetTextBlockPositions() const
{
    FTextBlockPositions Positions;
    Positions.ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot);
    Positions.ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot);
    
    // 동일한 절대 좌표 사용
    Positions.ScoreInitialPos = UScoreDisplayWidget::SCORE_TEXT_POS;
    Positions.ComboInitialPos = UScoreDisplayWidget::COMBO_TEXT_POS;
    
    return Positions;
}

FAnimationParameters UScoreWidgetAnimator::SetupAnimationParameters() const
{
    FAnimationParameters Params;
    Params.FadeDuration = 1.0f;
    Params.FadeInterval = 0.05f;
    Params.FadeSteps = FMath::RoundToInt(Params.FadeDuration / Params.FadeInterval);
    Params.FadeStep = 1.0f / Params.FadeSteps;
    Params.TotalMoveDistance = -100.0f;
    Params.MoveStep = Params.TotalMoveDistance / Params.FadeSteps;
    return Params;
}

void UScoreWidgetAnimator::StartFadeOutAnimation(UObject* WorldContextObject, float Delay)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World || !AreTextBlocksValid())
    {
        return;
    }
    
    // 먼저 이전 애니메이션 취소
    CancelAnimation();
    
    // 지연 후 애니메이션 실행
    bAnimationActive = true;
    World->GetTimerManager().SetTimer(
        AnimTimerHandle, 
        this, 
        &UScoreWidgetAnimator::ExecuteFadeOut, 
        Delay, 
        false
    );
}

// 기존 CancelAnimation 함수 수정
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

// 새 함수 추가: 텍스트 블록 속성 초기화
void UScoreWidgetAnimator::ResetTextBlockProperties()
{
    if (ScoreTextBlock)
    {
        // 색상 및 투명도 초기화
        FLinearColor OriginalColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        ScoreTextBlock->SetColorAndOpacity(OriginalColor);
        
        // 위치 초기화
        if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
        {
            // ScoreDisplayWidget의 상수 사용
            UScoreDisplayWidget* OwnerWidget = Cast<UScoreDisplayWidget>(GetOuter());
            if (OwnerWidget)
            {
                ScoreSlot->SetPosition(UScoreDisplayWidget::SCORE_TEXT_POS);
            }
        }
    }
    
    if (ComboMultiplierTextBlock)
    {
        // 색상 및 투명도 초기화
        FLinearColor OriginalColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        ComboMultiplierTextBlock->SetColorAndOpacity(OriginalColor);
        
        // 위치 초기화
        if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
        {
            UScoreDisplayWidget* OwnerWidget = Cast<UScoreDisplayWidget>(GetOuter());
            if (OwnerWidget)
            {
                ComboSlot->SetPosition(UScoreDisplayWidget::COMBO_TEXT_POS);
            }
        }
    }
    
    // 점수값도 초기화
    PendingScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
    bScoreTextActive = false;
}

void UScoreWidgetAnimator::ResetScoreValues()
{
    PendingScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
    
    if (ScoreTextBlock)
    {
        ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UScoreWidgetAnimator::ExecuteFadeOut()
{
    if (!AreTextBlocksValid())
    {
        return;
    }
    
    UWorld* World = ScoreTextBlock->GetWorld();
    if (!World) return;
    
    // 위치 및 애니메이션 설정
    FTextBlockPositions Positions = GetTextBlockPositions();
    FAnimationParameters AnimParams = SetupAnimationParameters();
    
    // 애니메이션 타이머 설정
    FTimerDelegate FadeDelegate = CreateFadeDelegate(Positions, AnimParams);
    World->GetTimerManager().SetTimer(
        AnimTimerHandle, 
        FadeDelegate, 
        AnimParams.FadeInterval, 
        true
    );
}

FTimerDelegate UScoreWidgetAnimator::CreateFadeDelegate(const FTextBlockPositions& Positions, const FAnimationParameters& Params)
{
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, Params, Positions]() mutable
    {
        static int32 CurrentStep = 0;
        CurrentStep++;
        
        // 애니메이션 값 계산
        float Alpha = 1.0f - (CurrentStep * Params.FadeStep);
        float CurrentMoveX = CurrentStep * Params.MoveStep;
        
        // 스코어 텍스트 업데이트
        if (ScoreTextBlock && Positions.ScoreSlot)
        {
            // 투명도 조절
            FLinearColor TextColor = ScoreTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            TextColor.A = Alpha;
            ScoreTextBlock->SetColorAndOpacity(TextColor);
            
            // 위치 이동
            FVector2D NewPos = Positions.ScoreInitialPos;
            NewPos.X += CurrentMoveX;
            Positions.ScoreSlot->SetPosition(NewPos);
        }
        
        // 콤보 텍스트 업데이트
        if (ComboMultiplierTextBlock && Positions.ComboSlot)
        {
            // 투명도 조절
            FLinearColor ComboColor = ComboMultiplierTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
            
            // 위치 이동
            FVector2D NewPos = Positions.ComboInitialPos;
            NewPos.X += CurrentMoveX;
            Positions.ComboSlot->SetPosition(NewPos);
        }
        
        // 애니메이션 종료
        if (CurrentStep >= Params.FadeSteps)
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
    
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    bAnimationActive = false;
}