// TitleLevelWidget.cpp

#include "TitleLevelWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"



void UTitleLevelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (TitleFadeBorder)
    {
        TitleFadeBorder->SetBrushColor(FLinearColor(0, 0, 0, 1));
        TitleFadeBorder->SetRenderOpacity(1.0f);

        if (UCanvasPanelSlot* BorderSlot = Cast<UCanvasPanelSlot>(TitleFadeBorder->Slot))
        {
            FVector2D ViewportSize = FVector2D(1920 * 3, 1080 * 2);
            BorderSlot->SetSize(ViewportSize);
            BorderSlot->SetPosition(FVector2D(-100, 0));
            BorderSlot->SetZOrder(20000);
        }
    }
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

void UTitleLevelWidget::PlayFadeOut(UBorder* TargetFadeBorder)
{
    if (!TargetFadeBorder)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayFadeOut: FadeBorder가 nullptr입니다."));
        return;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut: FadeBorder가 유효합니다."));}

    const float FadeDuration = FadeOutDuration;
    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);

    TWeakObjectPtr<UBorder> WeakBorder(TargetFadeBorder);
    FTimerHandle* FadeHandle = new FTimerHandle;
    UWorld* World = GetWorld();
    if (!World)
    {
        delete FadeHandle;
        delete Elapsed;
        return;
    }

    World->GetTimerManager().SetTimer(*FadeHandle, [WeakBorder, FadeDuration, TickInterval, Elapsed, FadeHandle, World]()
    {
        if (!WeakBorder.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut: WeakBorder가 유효하지 않습니다."));
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
            UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut: FadeOut 완료"));
            WeakBorder->SetRenderOpacity(0.0f);
            World->GetTimerManager().ClearTimer(*FadeHandle);
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

void UTitleLevelWidget::StartGame()
{
    if (TitleFadeBorder)
    {
        // 페이드 아웃 등 효과 처리
        PlayFadeOut(TitleFadeBorder);

        // 타이머 코드는 유지
        if (UWorld* World = GetWorld())
        {
            FTimerHandle GameStartHandle;
            TWeakObjectPtr<UTitleLevelWidget> WeakThis(this);
            World->GetTimerManager().SetTimer(
                GameStartHandle,
                [WeakThis]()
                {
                    if (WeakThis.IsValid())
                    {
                        UGameplayStatics::OpenLevel(WeakThis->GetWorld(), TEXT("PlayLevel"));
                        // RemoveFromParent()는 여기서 호출
                        WeakThis->RemoveFromParent();
                    }
                },
                FadeOutDuration,
                false
            );
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FadeBorder가 설정되지 않았다."));
    }
}