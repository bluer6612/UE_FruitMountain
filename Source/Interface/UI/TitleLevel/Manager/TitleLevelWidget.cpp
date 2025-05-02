#include "TitleLevelWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"
#include "Actors/FruitBall.h"

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
    if (!TitleFadeBorder)
    {
        UE_LOG(LogTemp, Error, TEXT("TitleFadeBorder가 아닙니다."));
        return;
    }

    const float FadeDuration = FadeOutDuration;
    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);

    TWeakObjectPtr<UTitleLevelWidget> WeakThis(this);
    TWeakObjectPtr<UBorder> WeakBorder(TitleFadeBorder);

    GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, [WeakThis, WeakBorder, FadeDuration, TickInterval, Elapsed, this]()
    {
        if (!WeakThis.IsValid() || !WeakBorder.IsValid())
        {
            if (UWorld* World = WeakThis.IsValid() ? WeakThis->GetWorld() : nullptr)
            {
                World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
            }
            delete Elapsed;
            return;
        }

        *Elapsed += TickInterval;
        float Alpha = 1.0f - FMath::Clamp(*Elapsed / FadeDuration, 0.f, 1.f);
        WeakBorder->SetRenderOpacity(Alpha);

        if (*Elapsed >= FadeDuration)
        {
            WeakBorder->SetRenderOpacity(0.0f);

            UWorld* World = WeakThis->GetWorld();
            if (World)
            {
                World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
            }

            // TitleLevel에서 페이드 아웃이 끝나면 StartLogoAndMenuFadeIn 호출
            if (World->GetMapName().Contains(TEXT("TitleLevel")) && !bLogoFadeInCalled)
            {
                if (UMainMenuWidget* MainMenu = Cast<UMainMenuWidget>(WeakThis.Get()))
                {
                    bLogoFadeInCalled = true;
                    MainMenu->StartLogoAndMenuFadeIn();
                }
            }

            delete Elapsed;
        }
    }, TickInterval, true);
}

void UTitleLevelWidget::MenuFadeOut(UImage* TargetImage, float Duration)
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
    if (!World) { delete FadeHandle; delete Elapsed; return; }

    World->GetTimerManager().SetTimer(*FadeHandle, [WeakImage, Duration, TickInterval, Elapsed, FadeHandle, World]()
    {
        if (!WeakImage.IsValid())
        {
            if (IsValid(World))
                World->GetTimerManager().ClearTimer(*FadeHandle);
            delete FadeHandle;
            delete Elapsed;
            return;
        }
        *Elapsed += TickInterval;
        float Alpha = 1.0f - FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
        WeakImage->SetRenderOpacity(Alpha);

        if (*Elapsed >= Duration)
        {
            WeakImage->SetRenderOpacity(0.0f);
            if (IsValid(World))
            {
                World->GetTimerManager().ClearTimer(*FadeHandle);
            }

            delete FadeHandle;
            delete Elapsed;
        }
    }, TickInterval, true);
}

void UTitleLevelWidget::MenuFadeOutMultiple(const TArray<UImage*>& Images, float Duration)
{
    for (UImage* Img : Images)
    {
        MenuFadeOut(Img, Duration);
    }
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
    UWorld* World = GetWorld();

    // 타이틀 용 미리보기 과일 제거
    if (World)
    {
        TArray<AActor*> PreviewFruits;
        UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), PreviewFruits);
        int32 RemovedCount = 0;
        for (AActor* FruitActor : PreviewFruits)
        {
            AFruitBall* Fruit = Cast<AFruitBall>(FruitActor);
            if (Fruit && Fruit->bIsPreviewBall)
            {
                Fruit->Destroy();
                ++RemovedCount;
            }
        }
        UE_LOG(LogTemp, Display, TEXT("타이틀 미리보기 과일 %d개 제거 완료"), RemovedCount);
    }

    if (TitleFadeBorder)
    {
        PlayFadeOut();

        if (World)
        {
            FTimerHandle GameStartHandle;
            TWeakObjectPtr<UTitleLevelWidget> WeakThis(this);
            World->GetTimerManager().SetTimer(
                GameStartHandle,
                [WeakThis, this]()
                {
                    if (!WeakThis.IsValid())
                    {
                        UE_LOG(LogTemp, Error, TEXT("WeakThis가 유효하지 않습니다! 클래스: "));
                        return;
                    }
                    if (WeakThis.IsValid())
                    {
                        // 모든 타이머 해제 (자기 자신 및 주요 위젯)
                        if (UWorld* World = WeakThis->GetWorld())
                        {
                            World->GetTimerManager().ClearTimer(WeakThis->FadeOutTimerHandle);
                            World->GetTimerManager().ClearAllTimersForObject(this);
                        }

                        UGameplayStatics::OpenLevel(WeakThis->GetWorld(), TEXT("PlayLevel"));
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
        UE_LOG(LogTemp, Error, TEXT("TitleFadeBorder가 nullptr입니다! 클래스: %s"), *GetClass()->GetName());
    }
}