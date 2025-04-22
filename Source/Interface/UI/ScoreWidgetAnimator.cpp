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
    if (!ScoreTextBlock || !ScoreTextBlock->GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("ScoreTextBlock이 유효하지 않습니다"));
        return;
    }

    // 디버그 로그 추가
    UE_LOG(LogTemp, Display, TEXT("StartFadeOutAnimation 호출됨, 딜레이: %.2f"), Delay);
    
    // 기존 애니메이션 취소
    CancelAnimation();
    
    // 위치 정보 직접 가져오기
    FVector2D ScorePos = UScoreDisplayWidget::SCORE_TEXT_POS;
    FVector2D ComboPos = UScoreDisplayWidget::COMBO_TEXT_POS;
    
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        ScorePos = ScoreSlot->GetPosition();
    }
    
    if (ComboMultiplierTextBlock && Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
    {
        ComboPos = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot)->GetPosition();
    }
    
    // 텍스트 블록 가시성 확인
    if (ScoreTextBlock->GetVisibility() != ESlateVisibility::HitTestInvisible &&
        ScoreTextBlock->GetVisibility() != ESlateVisibility::Visible)
    {
        ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
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
        
        UE_LOG(LogTemp, Display, TEXT("애니메이션 타이머 설정 완료, 타이머 핸들 유효: %s"), 
               AnimTimerHandle.IsValid() ? TEXT("예") : TEXT("아니오"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World가 null입니다"));
    }
}

void UScoreWidgetAnimator::CancelAnimation()
{
    // 현재 활성화된 애니메이션 타이머 정리
    if (bAnimationActive && IsValid(ScoreTextBlock) && ScoreTextBlock->GetWorld())
    {
        ScoreTextBlock->GetWorld()->GetTimerManager().ClearTimer(AnimTimerHandle);
        UE_LOG(LogTemp, Display, TEXT("진행 중인 애니메이션 취소"));
    }
    
    // 애니메이션 상태 초기화
    bAnimationActive = false;
    CurrentAnimStep = 0;
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
    FScoreAnimParams Params;
    
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, ScorePos, ComboPos, Params]() mutable
    {
        if (!ScoreTextBlock || !ScoreTextBlock->GetWorld())
        {
            UE_LOG(LogTemp, Error, TEXT("애니메이션 타이머: ScoreTextBlock이 유효하지 않음"));
            CancelAnimation();
            return;
        }
        
        CurrentAnimStep++;
        
        // 매개변수화된 상수 사용하여 애니메이션 값 계산
        float Alpha = 1.0f - (CurrentAnimStep * Params.AlphaStepSize);
        float CurrentMoveX = CurrentAnimStep * Params.MoveStepSize;
        
        // ScoreTextBlock 애니메이션
        FLinearColor TextColor = FLinearColor::White;
        TextColor.A = Alpha;
        ScoreTextBlock->SetColorAndOpacity(TextColor);
        
        if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
        {
            ScoreSlot->SetPosition(FVector2D(ScorePos.X - CurrentMoveX, ScorePos.Y));
        }
        
        // ComboMultiplierTextBlock 애니메이션 - IsVisible() 조건 제거
        if (ComboMultiplierTextBlock && !ComboMultiplierTextBlock->GetText().IsEmpty())
        {
            FLinearColor ComboColor = UScoreDisplayWidget::BRIGHT_YELLOW_COLOR;
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
            
            if (UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
            {
                ComboSlot->SetPosition(FVector2D(ComboPos.X - CurrentMoveX, ComboPos.Y));
            }
        }
        
        // 애니메이션 종료 체크
        if (CurrentAnimStep >= Params.TotalSteps)
        {
            UE_LOG(LogTemp, Display, TEXT("페이드 아웃 애니메이션 완료"));
            
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
    UE_LOG(LogTemp, Display, TEXT("ExecuteFadeOut 호출됨"));

    if (!ScoreTextBlock || !ScoreTextBlock->GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("ExecuteFadeOut: ScoreTextBlock 또는 World가 유효하지 않음"));
        return;
    }
    
    // 두 텍스트 블록 모두 동일한 가시성으로 설정 (중요)
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    // 콤보 텍스트가 비어있지 않은 경우에만 가시성 설정
    if (ComboMultiplierTextBlock && !ComboMultiplierTextBlock->GetText().IsEmpty())
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    // 위치 정보 직접 가져오기
    FVector2D ScorePos = UScoreDisplayWidget::SCORE_TEXT_POS;
    FVector2D ComboPos = UScoreDisplayWidget::COMBO_TEXT_POS;
    
    if (UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        ScorePos = ScoreSlot->GetPosition();
    }
    
    if (ComboMultiplierTextBlock && Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot))
    {
        ComboPos = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot)->GetPosition();
    }
    
    // 애니메이션 파라미터 설정
    FScoreAnimParams Params;
    CurrentAnimStep = 0;
    
    // 애니메이션 타이머 설정
    UWorld* World = ScoreTextBlock->GetWorld();
    FTimerDelegate FadeDelegate = CreateFadeDelegate(ScorePos, ComboPos);
    
    // 타이머 설정 - 프레임마다 호출
    World->GetTimerManager().SetTimer(
        AnimTimerHandle, 
        FadeDelegate, 
        Params.FrameInterval, 
        true
    );
    
    UE_LOG(LogTemp, Display, TEXT("페이드 애니메이션 시작: 총 단계 %d, 간격 %.3f초"), 
           Params.TotalSteps, Params.FrameInterval);
}

// ExecuteAnimationEnd() 함수 수정
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
    
    // 애니메이션 종료 델리게이트 호출
    OnAnimationEnd.Broadcast();
}