// TitleLevelWidget.cpp

#include "TitleLevelWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"

void UTitleLevelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // 필요시 초기화 코드
}

void UTitleLevelWidget::PlayFadeIn(UImage* TargetImage, float Duration)
{
    if (!TargetImage)
    {
        return;
    }

    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);
    FTimerHandle* FadeHandle = new FTimerHandle;

    TWeakObjectPtr<UImage> WeakImage(TargetImage);
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

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

void UTitleLevelWidget::PlayFadeOut()
{
    if (!FadeBorder)
    {
        return;
    }

    const float FadeDuration = FadeOutDuration;
    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);

    TWeakObjectPtr<UTitleLevelWidget> WeakThis(this);
    TWeakObjectPtr<UBorder> WeakBorder(FadeBorder);

    FTimerHandle* FadeHandle = new FTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(*FadeHandle, [WeakThis, WeakBorder, FadeDuration, TickInterval, Elapsed, FadeHandle, this]()
    {
        if (!WeakThis.IsValid() || !WeakBorder.IsValid())
        {
            if (UWorld* World = WeakThis.IsValid() ? WeakThis->GetWorld() : nullptr)
            {
                World->GetTimerManager().ClearTimer(*FadeHandle);
            }
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

            if (UWorld* World = WeakThis->GetWorld())
            {
                World->GetTimerManager().ClearTimer(*FadeHandle);
            }

            UWorld* World = GetWorld();
            if (World->GetMapName().Contains(TEXT("TitleLevel")))
            {
                // TitleLevel에서 페이드 아웃이 끝나면 StartLogoAndMenuFadeIn 호출
                if (UMainMenuWidget* MainMenu = Cast<UMainMenuWidget>(WeakThis.Get()))
                {
                    MainMenu->StartLogoAndMenuFadeIn();
                }
            }

            delete FadeHandle;
            delete Elapsed;
        }
    }, TickInterval, true);
}

bool UTitleLevelWidget::HandleMenuKey(const FKey& Key, int32& InOutIndex, int32 ItemCount, TFunction<void()> OnSelect)
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