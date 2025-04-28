#include "ComboCountWidgetAnimator.h"
#include "TimerManager.h"
#include "Engine/World.h"

UComboCountWidgetAnimator::UComboCountWidgetAnimator()
{
    ComboCountImage = nullptr;
    ComboTextBlock = nullptr;
    CurrentComboCount = 0;
    bFadingOut = false;
    FadeOutDuration = 0.5f;
}

void UComboCountWidgetAnimator::BeginDestroy()
{
    CancelAnimation();
    
    ComboCountImage = nullptr;
    ComboTextBlock = nullptr;
    
    Super::BeginDestroy();
}

void UComboCountWidgetAnimator::Initialize(UImage* InComboCountImage, UTextBlock* InComboTextBlock)
{
    ComboCountImage = InComboCountImage;
    ComboTextBlock = InComboTextBlock;
}

// ComboCountWidget에서 이동된 함수
void UComboCountWidgetAnimator::UpdateComboCount(int32 NewComboCount)
{
    // 콤보가 새로 시작될 때 애니메이션 상태 초기화
    if (bFadingOut)
    {
        CancelAnimation();
    }

    CurrentComboCount = NewComboCount;

    if (CurrentComboCount >= 2)
    {
        SetComboCountVisibility(true);

        // 투명도 복구
        if (ComboTextBlock)
        {
            ComboTextBlock->SetRenderOpacity(1.0f);
        }
        if (ComboCountImage)
        {
            ComboCountImage->SetRenderOpacity(1.0f);
        }
        if (ComboTextBlock)
        {
            ComboTextBlock->SetText(FText::AsNumber(CurrentComboCount));
        }
    }
    else
    {
        SetComboCountVisibility(false);
    }
}

void UComboCountWidgetAnimator::ResetComboCount()
{
    if (CurrentComboCount >= 2)
    {
        PlayFadeOutAnimation();
    }
    else
    {
        SetComboCountVisibility(false);
    }
    CurrentComboCount = 0;
}

void UComboCountWidgetAnimator::SetComboCountVisibility(bool bVisible)
{
    ESlateVisibility InVisibility = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;

    if (ComboCountImage)
    {
        ComboCountImage->SetVisibility(InVisibility);
    }

    if (ComboTextBlock)
    {
        ComboTextBlock->SetVisibility(InVisibility);
    }
}

void UComboCountWidgetAnimator::CancelAnimation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (bFadingOut)
    {
        World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
        bFadingOut = false;
    }

    if (ComboCountImage && ComboTextBlock)
    {
        ComboCountImage->SetRenderOpacity(1.0f);
        ComboTextBlock->SetRenderOpacity(1.0f);
    }
}

void UComboCountWidgetAnimator::ExecuteFadeOutStep()
{
    static float ElapsedTime = 0.0f;
    
    UWorld* World = GetWorld();
    if (!World || !ComboCountImage || !ComboTextBlock)
        return;
        
    ElapsedTime += 0.016f;
    
    // 진행률 계산 (0.0 ~ 1.0)
    float Progress = FMath::Clamp(ElapsedTime / FadeOutDuration, 0.0f, 1.0f);
    
    // 불투명도 계산 (1.0 -> 0.0)
    float CurrentOpacity = 1.0f - Progress;
    
    // 위젯에 적용
    ComboTextBlock->SetRenderOpacity(CurrentOpacity);
    ComboCountImage->SetRenderOpacity(CurrentOpacity);
    
    // 페이드 아웃 완료
    if (Progress >= 1.0f)
    {
        // 타이머 중지
        World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
        
        // 상태 초기화
        ElapsedTime = 0.0f;
        bFadingOut = false;
        
        // 애니메이션 완료 후 위젯 숨기기
        SetComboCountVisibility(false);
        
        // 애니메이션 완료 이벤트 발생
        OnAnimationComplete.ExecuteIfBound();
        
        //UE_LOG(LogTemp, Display, TEXT("콤보 카운트 페이드 아웃 애니메이션 완료"));
    }
}

void UComboCountWidgetAnimator::PlayFadeOutAnimation()
{
    if (bFadingOut)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World || !ComboCountImage || !ComboTextBlock)
    {
        return;
    }

    bFadingOut = true;

    // 타이머를 이용해 페이드 아웃 단계적으로 실행
    World->GetTimerManager().SetTimer(
        FadeOutTimerHandle,
        this,
        &UComboCountWidgetAnimator::ExecuteFadeOutStep,
        0.005f,
        true
    );
}