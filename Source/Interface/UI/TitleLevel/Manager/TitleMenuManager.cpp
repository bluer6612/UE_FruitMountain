#include "TitleMenuManager.h"
#include "MenuIndicatorAnimator.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "TimerManager.h"

void UTitleMenuManager::InitializeIndicator(UImage* InSelectIndicator)
{
    if (!IndicatorAnimator)
        IndicatorAnimator = NewObject<UMenuIndicatorAnimator>(this);

    if (IndicatorAnimator)
        IndicatorAnimator->Initialize(InSelectIndicator);
}

void UTitleMenuManager::MoveIndicatorTo(const FVector2D& NewPosition)
{
    if (IndicatorAnimator)
    {
        IndicatorAnimator->MoveToPosition(NewPosition);

        UE_LOG(LogTemp, Warning, TEXT("IndicatorAnimator가 초기화 됨됨."));

    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("IndicatorAnimator가 초기화되지 않았습니다."));
    }
}

bool UTitleMenuManager::HandleMenuKey(const FKey& Key, int32& InOutIndex, int32 ItemCount, TFunction<void()> OnSelect)
{
    if (Key == EKeys::Up || Key == EKeys::W)
    {
        InOutIndex = (InOutIndex - 1 + ItemCount) % ItemCount;
        return true;
    }
    if (Key == EKeys::Down || Key == EKeys::S)
    {
        InOutIndex = (InOutIndex + 1) % ItemCount;
        return true;
    }
    if (Key == EKeys::Enter || Key == EKeys::SpaceBar)
    {
        if (OnSelect) OnSelect();
        return true;
    }
    return false;
}

void UTitleMenuManager::PlayFadeIn(UImage* TargetImage, UObject* WorldContext, float Duration)
{
    if (!TargetImage || !WorldContext) return;

    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);
    FTimerHandle* FadeHandle = new FTimerHandle;

    TWeakObjectPtr<UImage> WeakImage(TargetImage);

    UWorld* World = WorldContext->GetWorld();
    if (!World) return;

    World->GetTimerManager().SetTimer(*FadeHandle, [WeakImage, Duration, TickInterval, Elapsed, FadeHandle, World]()
    {
        if (!WeakImage.IsValid())
        {
            World->GetTimerManager().ClearTimer(*FadeHandle);
            delete FadeHandle;
            delete Elapsed;
            return;
        }
        *Elapsed += TickInterval;
        float Alpha = FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
        WeakImage->SetRenderOpacity(Alpha);

        if (Alpha >= 1.f)
        {
            World->GetTimerManager().ClearTimer(*FadeHandle);
            delete FadeHandle;
            delete Elapsed;
        }
    }, TickInterval, true);
}

void UTitleMenuManager::PlayFadeOut(UBorder* InFadeBorder, UObject* WorldContext, float FadeDuration)
{
    if (!InFadeBorder || !WorldContext) return;

    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);

    TWeakObjectPtr<UBorder> WeakBorder(FadeBorder);

    FTimerHandle* FadeHandle = new FTimerHandle;
    UWorld* World = WorldContext->GetWorld();
    if (!World) return;

    World->GetTimerManager().SetTimer(*FadeHandle, [WeakBorder, FadeDuration, TickInterval, Elapsed, FadeHandle, World]()
    {
        if (!WeakBorder.IsValid())
        {
            World->GetTimerManager().ClearTimer(*FadeHandle);
            delete FadeHandle;
            delete Elapsed;
            return;
        }

        *Elapsed += TickInterval;
        float Alpha = 1.0f - FMath::Clamp(*Elapsed / FadeDuration, 0.f, 1.f);
        WeakBorder->SetRenderOpacity(Alpha);

        if (*Elapsed >= FadeDuration)
        {
            WeakBorder->SetRenderOpacity(0.0f);
            World->GetTimerManager().ClearTimer(*FadeHandle);
            delete FadeHandle;
            delete Elapsed;
        }
    }, TickInterval, true);
}