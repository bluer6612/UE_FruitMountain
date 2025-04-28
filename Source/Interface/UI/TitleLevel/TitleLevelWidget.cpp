#include "TitleLevelWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/Border.h"

UTitleLevelWidget::UTitleLevelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
}

void UTitleLevelWidget::InitializeTitleWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 진입"));

    UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!RootCanvas)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        WidgetTree->RootWidget = RootCanvas;
    }

    // 1. 기존 Renderer 활용해서 이미지 생성
    UUIWidgetRenderer* Renderer = UUIWidgetRenderer::GetInstance();
    if (Renderer)
    {
        LogoImage = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Logo,
            TEXT("/Game/UI/TitleLevel/UI_Title_Logo"),
            FVector2D(633.f, 369.f), 150.f, 267.5f);
        LogoImage->SetRenderOpacity(0.f);

        MenuImage = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Menu,
            TEXT("/Game/UI/TitleLevel/UI_Title_Menu"),
            FVector2D(592.f, 359.f), 150.f, 35.f);
        MenuImage->SetRenderOpacity(0.f);
    }

    if (!LogoImage)
    {
        UE_LOG(LogTemp, Error, TEXT("LogoImage가 nullptr입니다!"));
    }
    if (!MenuImage)
    {
        UE_LOG(LogTemp, Error, TEXT("MenuImage가 nullptr입니다!"));
    }

    // 2. FadeBorder 인라인 직접 생성 및 추가
    if (!FadeBorder)
    {
        FadeBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FadeBorder"));
        FadeBorder->SetBrushColor(FLinearColor::Black);
        FadeBorder->SetRenderOpacity(1.f);

        RootCanvas->AddChild(FadeBorder);
        if (UCanvasPanelSlot* InSlot = Cast<UCanvasPanelSlot>(FadeBorder->Slot))
        {
            InSlot->SetAnchors(FAnchors(0, 0, 1, 1));
            InSlot->SetOffsets(FMargin(0));
            InSlot->SetZOrder(20000);
        }
        UE_LOG(LogTemp, Warning, TEXT("FadeBorder 생성 및 추가 완료"));
    }

    // 3. 페이드 효과 실행
    PlayFadeOut(FadeBorder, FadeOutDuration);

    if (LogoImage)
    {
        FTimerHandle LogoFadeHandle;
        GetWorld()->GetTimerManager().SetTimer(LogoFadeHandle, [this]()
        {
            PlayFadeIn(LogoImage);
        }, FadeOutDuration + 0.5f, false); 
        UE_LOG(LogTemp, Warning, TEXT("LogoImage 페이드인 타이머 등록"));
    }

    if (MenuImage)
    {
        FTimerHandle MenuFadeHandle;
        GetWorld()->GetTimerManager().SetTimer(MenuFadeHandle, [this]()
        {
            PlayFadeIn(MenuImage);
        }, FadeOutDuration + 1.0f, false); 
    }

    UpdateMenuSelection();
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 종료"));
}

// 검은 화면 페이드아웃 함수
void UTitleLevelWidget::PlayFadeOut(UBorder* TargetBorder, float Duration)
{
    if (!TargetBorder)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayFadeOut: TargetBorder가 nullptr입니다!"));
        return;
    }

    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);

    TWeakObjectPtr<UBorder> WeakBorder(TargetBorder);
    TWeakObjectPtr<UTitleLevelWidget> WeakThis(this);

    FTimerHandle* FadeHandle = new FTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(*FadeHandle, [WeakBorder, WeakThis, Duration, TickInterval, Elapsed, FadeHandle]()
    {
        if (!WeakThis.IsValid() || !WeakBorder.IsValid())
        {
            if (FadeHandle)
            {
                if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()))
                {
                    World->GetTimerManager().ClearTimer(*FadeHandle);
                }
                delete FadeHandle;
            }
            delete Elapsed;
            return;
        }

        *Elapsed += TickInterval;
        float Alpha = 1.f - FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
        WeakBorder->SetRenderOpacity(Alpha);

        UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut: Alpha = %f"), Alpha);

        if (Alpha <= 0.f)
        {
            WeakBorder->SetRenderOpacity(0.f);
            WeakBorder->RemoveFromParent();
            if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()))
            {
                World->GetTimerManager().ClearTimer(*FadeHandle);
            }
            delete FadeHandle;
            delete Elapsed;
            UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut: FadeBorder 제거 완료"));
        }
    }, TickInterval, true);
}

void UTitleLevelWidget::PlayFadeIn(UImage* TargetImage)
{
    if (!TargetImage)
    {
        return;
    }

    const float FadeDuration = 0.25f;
    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);

    TWeakObjectPtr<UTitleLevelWidget> WeakThis(this);
    TWeakObjectPtr<UImage> WeakImage(TargetImage);

    FTimerHandle* FadeHandle = new FTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(*FadeHandle, [WeakThis, WeakImage, FadeDuration, TickInterval, Elapsed, FadeHandle]()
    {
        if (!WeakThis.IsValid() || !WeakImage.IsValid())
        {
            if (FadeHandle)
            {
                if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()))
                {
                    World->GetTimerManager().ClearTimer(*FadeHandle);
                }
                delete FadeHandle;
            }
            delete Elapsed;
            return;
        }

        *Elapsed += TickInterval;
        float Alpha = FMath::Clamp(*Elapsed / FadeDuration, 0.f, 1.f);
        WeakImage->SetRenderOpacity(Alpha);

        if (Alpha >= 1.f)
        {
            if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()))
            {
                World->GetTimerManager().ClearTimer(*FadeHandle);
            }
            delete FadeHandle;
            delete Elapsed;
        }
    }, TickInterval, true);
}

FReply UTitleLevelWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::Up || Key == EKeys::W)
    {
        CurrentMenuIndex = FMath::Max(0, CurrentMenuIndex - 1);
        UpdateMenuSelection();
        return FReply::Handled();
    }
    if (Key == EKeys::Down || Key == EKeys::S)
    {
        CurrentMenuIndex = FMath::Min(1, CurrentMenuIndex + 1);
        UpdateMenuSelection();
        return FReply::Handled();
    }
    if (Key == EKeys::SpaceBar || Key == EKeys::Enter)
    {
        OnMenuSelect();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTitleLevelWidget::UpdateMenuSelection()
{
    // 메뉴 선택 효과(예: 밝기, 크기 등) 구현 필요
}

void UTitleLevelWidget::OnMenuSelect()
{
    if (CurrentMenuIndex == 0)
    {
        UGameplayStatics::OpenLevel(this, TEXT("PlayLevel"));
    }
    else if (CurrentMenuIndex == 1)
    {
        UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
    }
}