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
    CurrentComboCount = NewComboCount;
    
    // 콤보가 2 이상일 때만 보이게
    if (CurrentComboCount >= 2)
    {
        SetComboCountVisibility(true);
        
        // 텍스트 업데이트
        if (ComboTextBlock)
        {
            ComboTextBlock->SetText(FText::AsNumber(CurrentComboCount));
        }
    }
}

// ComboCountWidget에서 이동된 함수
void UComboCountWidgetAnimator::ResetComboCount()
{
    if (CurrentComboCount >= 2)
    {
        // 페이드 아웃 애니메이션 시작
        PlayFadeOutAnimation();
    }

    CurrentComboCount = 0;
}

// ComboCountWidget에서 이동된 함수
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

void UComboCountWidgetAnimator::PlayFadeOutAnimation()
{
    // 이미 페이드 아웃 중이면 중복 실행 방지
    if (bFadingOut || !ComboCountImage || !ComboTextBlock)
        return;
        
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    bFadingOut = true;
    
    // 초기 상태 설정
    ComboTextBlock->SetRenderOpacity(1.0f);
    ComboCountImage->SetRenderOpacity(1.0f);
    
    // 타이머로 페이드 아웃 실행
    FTimerDelegate FadeOutDelegate;
    FadeOutDelegate.BindUObject(this, &UComboCountWidgetAnimator::ExecuteFadeOutStep);
    
    World->GetTimerManager().SetTimer(
        FadeOutTimerHandle, 
        FadeOutDelegate, 
        0.016f, // ~60fps
        true
    );
    
    UE_LOG(LogTemp, Display, TEXT("콤보 카운트 페이드 아웃 애니메이션 시작"));
}

void UComboCountWidgetAnimator::CancelAnimation()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
        
    if (bFadingOut)
    {
        World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
        bFadingOut = false;
    }
    
    // 위젯이 유효하면 초기 상태로 재설정
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
        
        UE_LOG(LogTemp, Display, TEXT("콤보 카운트 페이드 아웃 애니메이션 완료"));
    }
}