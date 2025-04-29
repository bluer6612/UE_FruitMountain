#include "MenuIndicatorAnimator.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

UMenuIndicatorAnimator::UMenuIndicatorAnimator()
{
    // 초기화
}

void UMenuIndicatorAnimator::Initialize(UImage* InIndicator)
{
    Indicator = InIndicator;
    
    if (Indicator)
    {
        Indicator->SetRenderOpacity(1.0f);
        
        // 0.25초 후 애니메이션 시작
        if (UWorld* World = Indicator->GetWorld())
        {
            FTimerHandle FirstAnimHandle;
            World->GetTimerManager().SetTimer(FirstAnimHandle, [this]()
            {
                PlayAnimation();
            }, 0.25f, false);
        }
    }
}

void UMenuIndicatorAnimator::StartAnimation(bool bStart)
{
    bIsAnimating = bStart;
    
    if (bStart && !bIsAnimationRunning)
    {
        PlayAnimation();
    }
    else if (!bStart)
    {
        ClearAnimationTimers();
    }
}

void UMenuIndicatorAnimator::MoveToPosition(const FVector2D& NewPosition)
{
    if (!Indicator)
    {
        return;
    }
    
    // 애니메이션 중단
    ClearAnimationTimers();
    bIsAnimationRunning = false;
    
    // 인디케이터 위치 변경
    if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(Indicator->Slot))
    {
        IndicatorSlot->SetPosition(NewPosition);
    }
    
    // 색상 초기화
    Indicator->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f));
    Indicator->SetRenderOpacity(1.0f);
    
    // 0.5초 후 애니메이션 재개
    if (UWorld* World = Indicator->GetWorld())
    {
        FTimerHandle ResetAnimHandle;
        AnimationTimerHandles.Add(ResetAnimHandle);
        
        World->GetTimerManager().SetTimer(
            ResetAnimHandle, 
            [this]() {
                bIsAnimating = true;
                PlayAnimation();
            }, 
            0.5f,
            false
        );
    }
}

void UMenuIndicatorAnimator::ClearAnimationTimers()
{
    if (!Indicator || !Indicator->GetWorld())
    {
        return;
    }
    
    UWorld* World = Indicator->GetWorld();
    
    // 메인 애니메이션 타이머 정리
    World->GetTimerManager().ClearTimer(AnimationTimerHandle);
    
    // 추가 타이머 정리
    for (FTimerHandle& Handle : AnimationTimerHandles)
    {
        World->GetTimerManager().ClearTimer(Handle);
    }
    AnimationTimerHandles.Empty();
}

void UMenuIndicatorAnimator::BeginDestroy()
{
    ClearAnimationTimers();
    Super::BeginDestroy();
}

void UMenuIndicatorAnimator::PlayAnimation()
{
    bIsAnimationRunning = true;
    
    if (!Indicator)
    {
        bIsAnimationRunning = false;
        return;
    }
    
    UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(Indicator->Slot);
    if (!IndicatorSlot)
    {
        bIsAnimationRunning = false;
        return;
    }
    
    const float AnimDuration = AnimationDuration;
    const float TickInterval = 0.006f;
    float* ElapsedTime = new float(0.0f);
    
    TWeakObjectPtr<UMenuIndicatorAnimator> WeakThis(this);
    TWeakObjectPtr<UImage> WeakIndicator(Indicator);
    
    // 애니메이션 시작 시 인디케이터의 원래 위치를 저장
    FVector2D BasePosition = IndicatorSlot->GetPosition();
    
    if (UWorld* World = Indicator->GetWorld())
    {
        World->GetTimerManager().ClearTimer(AnimationTimerHandle);
        
        World->GetTimerManager().SetTimer(
            AnimationTimerHandle, 
            [this, WeakThis, WeakIndicator, AnimDuration, TickInterval, ElapsedTime, BasePosition]()
            {
                if (!WeakThis.IsValid() || !WeakIndicator.IsValid())
                {
                    delete ElapsedTime;
                    return;
                }
                
                *ElapsedTime += TickInterval;
                float Progress = FMath::Clamp(*ElapsedTime / AnimDuration, 0.0f, 1.0f);
                
                // 왼쪽으로 이동하는 구간 (13.33%)
                if (Progress <= 0.1333f)
                {
                    float NormalizedProgress = Progress / 0.1333f;
                    float EasedProgress = 1.0f - FMath::Pow(1.0f - NormalizedProgress, 2.0f);
                    
                    if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                    {
                        float NewX = BasePosition.X - 15.f * EasedProgress;
                        IndicatorSlot->SetPosition(FVector2D(NewX, BasePosition.Y));
                    }
                    
                    // 왼쪽으로 이동 시 반짝임 한 번
                    float FlashCurve;
                    if (NormalizedProgress < 0.5f) {
                        FlashCurve = NormalizedProgress * 2.0f;
                    } else {
                        FlashCurve = 2.0f - (NormalizedProgress * 2.0f);
                    }
                    
                    float Brightness = 1.0f + 0.3f * FlashCurve;
                    WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
                    
                    float Opacity = 0.7f + 0.3f * FlashCurve;
                    WeakIndicator->SetRenderOpacity(Opacity);
                }
                // 오른쪽으로 돌아오는 구간 (23.33%)
                else if (Progress <= 0.3666f)
                {
                    float NormalizedProgress = (Progress - 0.1333f) / 0.2333f;
                    float EasedProgress = NormalizedProgress * NormalizedProgress;
                    
                    if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                    {
                        float NewX = BasePosition.X - 15.f + 15.f * EasedProgress;
                        IndicatorSlot->SetPosition(FVector2D(NewX, BasePosition.Y));
                    }
                    
                    float BellCurve = FMath::Sin(NormalizedProgress * PI);
                    
                    float Brightness = 1.0f + 0.2f * BellCurve;
                    WeakIndicator->SetColorAndOpacity(FLinearColor(Brightness, Brightness, Brightness));
                    
                    float Opacity = 0.8f + 0.2f * BellCurve;
                    WeakIndicator->SetRenderOpacity(Opacity);
                }
                // 대기 구간 및 종료 처리 (기존 코드 유지)
                else if (Progress <= 0.4666f)
                {
                    // 원래 위치로 설정
                    if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(WeakIndicator->Slot))
                    {
                        IndicatorSlot->SetPosition(BasePosition);
                    }
                    
                    // 기본 값
                    WeakIndicator->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f));
                    
                    // 대기 시간 동안 한 번의 미세한 반짝임
                    float IdleProgress = (Progress - 0.3666f) / 0.1f;
                    float PulseCurve = IdleProgress < 0.5f ? 
                                      (IdleProgress * 2.0f) : 
                                      (2.0f - IdleProgress * 2.0f);
                    
                    float IdleOpacity = 0.9f + 0.1f * PulseCurve;
                    WeakIndicator->SetRenderOpacity(IdleOpacity);
                }
                else
                {
                    // 애니메이션 조기 완료
                    if (UWorld* TimerWorld = WeakIndicator->GetWorld())
                    {
                        TimerWorld->GetTimerManager().ClearTimer(WeakThis->AnimationTimerHandle);
                    }
                    
                    delete ElapsedTime;
                    
                    if (WeakThis.IsValid())
                    {
                        WeakThis->bIsAnimationRunning = false;
                        
                        if (WeakThis->bIsAnimating)
                        {
                            WeakThis->PlayAnimation();
                        }
                    }
                    
                    return;
                }
                
                // 애니메이션 완료
                if (Progress >= 1.0f)
                {
                    if (UWorld* TimerWorld = WeakIndicator->GetWorld())
                    {
                        TimerWorld->GetTimerManager().ClearTimer(WeakThis->AnimationTimerHandle);
                    }
                    
                    delete ElapsedTime;
                    
                    if (WeakThis.IsValid())
                    {
                        WeakThis->bIsAnimationRunning = false;
                        
                        if (WeakThis->bIsAnimating)
                        {
                            WeakThis->PlayAnimation();
                        }
                    }
                }
            },
            TickInterval,
            true
        );
    }
}