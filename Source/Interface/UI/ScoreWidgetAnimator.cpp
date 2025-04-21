#include "ScoreWidgetAnimator.h"
#include "Components/CanvasPanelSlot.h"
#include "TimerManager.h"
#include "Engine/World.h"

UScoreWidgetAnimator::UScoreWidgetAnimator()
{
    ScoreTextBlock = nullptr;
    ComboMultiplierTextBlock = nullptr;
    PendingScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
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
    Positions.ScoreInitialPos = Positions.ScoreSlot ? Positions.ScoreSlot->GetPosition() : FVector2D(750.0f, 100.0f);
    Positions.ComboInitialPos = Positions.ComboSlot ? Positions.ComboSlot->GetPosition() : FVector2D(800.0f, 200.0f);
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
    
    bAnimationActive = false;
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
            
            // 텍스트 숨기기 및 원래 상태로 복원
            ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
            ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
            
            // 원래 위치 복원
            if (Positions.ScoreSlot) Positions.ScoreSlot->SetPosition(Positions.ScoreInitialPos);
            if (Positions.ComboSlot) Positions.ComboSlot->SetPosition(Positions.ComboInitialPos);
            
            // 불투명도 복원
            ScoreTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
            ComboMultiplierTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
            
            bAnimationActive = false;
            PendingScoreGain = 0;
        }
    });
    
    return FadeDelegate;
}