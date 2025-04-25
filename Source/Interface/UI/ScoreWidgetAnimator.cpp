#include "ScoreWidgetAnimator.h"
#include "Components/CanvasPanelSlot.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "ScoreDisplayWidget.h"

UScoreWidgetAnimator::UScoreWidgetAnimator()
{
    ScoreTextBlock_C = nullptr;
    ComboMultiplierTextBlock_C = nullptr;
    bAnimationActive = false;
    CurrentAnimStep = 0;
}

void UScoreWidgetAnimator::BeginDestroy()
{
    CancelAnimation();
    
    ScoreTextBlock_C = nullptr;
    ComboMultiplierTextBlock_C = nullptr;
    
    Super::BeginDestroy();
}

void UScoreWidgetAnimator::Initialize(UTextBlock* InScoreTextBlock_C, UTextBlock* InComboMultiplierTextBlock_C)
{
    // 텍스트 블록 참조 저장
    ScoreTextBlock_C = InScoreTextBlock_C;
    ComboMultiplierTextBlock_C = InComboMultiplierTextBlock_C;
    
    // 초기화 로그
    UE_LOG(LogTemp, Display, TEXT("ScoreWidgetAnimator: 텍스트 블록 초기화 완료"));
    
    // 초기 상태 - 텍스트 숨김
    if (ScoreTextBlock_C)
    {
        ScoreTextBlock_C->SetVisibility(ESlateVisibility::Hidden);
    }
    
    if (ComboMultiplierTextBlock_C)
    {
        ComboMultiplierTextBlock_C->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 기타 필요한 초기화
    bAnimationActive = false;
}

void UScoreWidgetAnimator::SetTextBlocks(UTextBlock* InScoreText, UTextBlock* InComboText)
{
    ScoreTextBlock_C = InScoreText;
    ComboMultiplierTextBlock_C = InComboText;
}

void UScoreWidgetAnimator::StartFadeOutAnimation(UObject* WorldContextObject, float Delay)
{
    // 유효성 검사
    if (!ScoreTextBlock_C || !ScoreTextBlock_C->GetWorld())
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
    if (ScoreTextBlock_C && ScoreTextBlock_C->GetWorld())
    {
        UWorld* World = ScoreTextBlock_C->GetWorld();
        World->GetTimerManager().ClearTimer(DelayTimerHandle);
        World->GetTimerManager().ClearTimer(AnimTimerHandle);
        
        // 위치 복원
        if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock_C->Slot))
        {
            ScoreSlot->SetPosition(UScoreDisplayWidget::SCORE_TEXT_POS);
        }
        
        if (ComboMultiplierTextBlock_C)
        {
            if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock_C->Slot))
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
        if (!ScoreTextBlock_C || !ScoreTextBlock_C->GetWorld())
        {
            CancelAnimation();
            return;
        }
        
        CurrentAnimStep++;
        
        // 페이드아웃 계산
        float Alpha = FMath::Max(1.0f - (CurrentAnimStep * Params.AlphaStepSize), 0.0f);
        float CurrentMoveX = CurrentAnimStep * Params.MoveStepSize;
        
        // ScoreTextBlock_C 애니메이션
        FSlateColor CurrentScoreColor = ScoreTextBlock_C->GetColorAndOpacity();
        FLinearColor ScoreColor = CurrentScoreColor.GetSpecifiedColor();
        ScoreColor.A = Alpha;
        ScoreTextBlock_C->SetColorAndOpacity(FSlateColor(ScoreColor));
        
        if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock_C->Slot))
        {
            ScoreSlot->SetPosition(FVector2D(ScorePos.X - CurrentMoveX, ScorePos.Y));
        }
        
        // ComboMultiplierTextBlock_C 애니메이션
        if (ComboMultiplierTextBlock_C)
        {
            FSlateColor CurrentComboColor = ComboMultiplierTextBlock_C->GetColorAndOpacity();
            FLinearColor ComboColor = CurrentComboColor.GetSpecifiedColor();
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock_C->SetColorAndOpacity(FSlateColor(ComboColor));
            
            if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock_C->Slot))
            {
                ComboSlot->SetPosition(FVector2D(ComboPos.X - CurrentMoveX, ComboPos.Y));
            }
        }
        
        // 애니메이션 종료 체크
        if (CurrentAnimStep >= Params.TotalSteps)
        {
            // 가시성으로 즉시 숨김
            ScoreTextBlock_C->SetVisibility(ESlateVisibility::Hidden);
            if (ComboMultiplierTextBlock_C)
            {
                ComboMultiplierTextBlock_C->SetVisibility(ESlateVisibility::Hidden);
            }
            
            // 타이머 중지
            ScoreTextBlock_C->GetWorld()->GetTimerManager().ClearTimer(AnimTimerHandle);
            
            // 애니메이션 종료 처리
            ExecuteAnimationEnd();
        }
    });
    
    return FadeDelegate;
}

void UScoreWidgetAnimator::ExecuteFadeOut()
{
    if (!ScoreTextBlock_C || !ScoreTextBlock_C->GetWorld())
    {
        return;
    }
    
    // 텍스트 블록 가시성 설정
    ScoreTextBlock_C->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    if (ComboMultiplierTextBlock_C)
    {
        ComboMultiplierTextBlock_C->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
        
    // 항상 정적 위치 상수 사용
    FVector2D ScorePos = UScoreDisplayWidget::SCORE_TEXT_POS;
    FVector2D ComboPos = UScoreDisplayWidget::COMBO_TEXT_POS;
    
    // 위치 강제 설정
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock_C->Slot))
    {
        ScoreSlot->SetPosition(ScorePos);
    }
    
    if (ComboMultiplierTextBlock_C && Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock_C->Slot))
    {
        Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock_C->Slot)->SetPosition(ComboPos);
    }
    
    // 애니메이션 파라미터 설정
    CurrentAnimStep = 0;
    
    // 타이머 설정
    UWorld* World = ScoreTextBlock_C->GetWorld();
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
    if (!ScoreTextBlock_C)
    {
        return;
    }
    
    // 항상 기본 정적 위치로 리셋
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock_C->Slot))
    {
        ScoreSlot->SetPosition(UScoreDisplayWidget::SCORE_TEXT_POS);
    }
    
    // 텍스트 설정
    FString ScoreText = FString::Printf(TEXT("+%d"), Score);
    ScoreTextBlock_C->SetText(FText::FromString(ScoreText));
    ScoreTextBlock_C->SetColorAndOpacity(UScoreDisplayWidget::SCORE_YELLOW_COLOR);
    ScoreTextBlock_C->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UScoreWidgetAnimator::AnimateComboText(int32 ComboCount, float ComboMultiplier, float Delay)
{
    if (!ComboMultiplierTextBlock_C)
    {
        return;
    }
    
    // 항상 기본 정적 위치로 리셋
    if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock_C->Slot))
    {
        ComboSlot->SetPosition(UScoreDisplayWidget::COMBO_TEXT_POS);
    }
    
    // 콤보가 없으면 표시하지 않음
    if (ComboCount < 2)
    {
        ComboMultiplierTextBlock_C->SetText(FText::GetEmpty());
        ComboMultiplierTextBlock_C->SetVisibility(ESlateVisibility::Hidden);
        return;
    }
    
    // 텍스트 설정
    FString ComboText = FString::Printf(TEXT("x%.1f"), ComboMultiplier);
    ComboMultiplierTextBlock_C->SetText(FText::FromString(ComboText));
    ComboMultiplierTextBlock_C->SetVisibility(ESlateVisibility::HitTestInvisible);
    ComboMultiplierTextBlock_C->SetColorAndOpacity(UScoreDisplayWidget::SCORE_YELLOW_COLOR);
}

void UScoreWidgetAnimator::FadeOutBoth(float Delay)
{
    if (ScoreTextBlock_C && ScoreTextBlock_C->GetWorld())
    {
        StartFadeOutAnimation(ScoreTextBlock_C->GetWorld(), Delay);
    }
}

void UScoreWidgetAnimator::ExecuteAnimationEnd()
{
    // 두 텍스트 모두 즉시 숨김
    if (ScoreTextBlock_C)
    {
        ScoreTextBlock_C->SetVisibility(ESlateVisibility::Hidden);
    }
    
    if (ComboMultiplierTextBlock_C)
    {
        ComboMultiplierTextBlock_C->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 애니메이션 종료 처리
    bAnimationActive = false;
    
    // 콜백 호출
    if (OnAnimationComplete.IsBound())
    {
        OnAnimationComplete.Execute();
    }
}