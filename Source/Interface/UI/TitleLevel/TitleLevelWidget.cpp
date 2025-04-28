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

    // Renderer가 이미지를 RootCanvas에 추가해두었으므로, 이름으로 찾아서 포인터를 저장
    LogoImage = nullptr;
    MenuImage = nullptr;
    TArray<UWidget*> Widgets;
    WidgetTree->GetAllWidgets(Widgets);
    for (UWidget* Widget : Widgets)
    {
        if (Widget->GetFName() == TEXT("LogoImage"))
        {
            LogoImage = Cast<UImage>(Widget);
        }
        else if (Widget->GetFName() == TEXT("MenuImage"))
        {
            MenuImage = Cast<UImage>(Widget);
        }
    }

    // 처음엔 모두 투명하게
    if (LogoImage)
    {
        LogoImage->SetRenderOpacity(0.f);
    }
    if (MenuImage)
    {
        MenuImage->SetRenderOpacity(0.f);
    }

    // 1.5초간 검은 화면 페이드아웃 효과용 Border 생성
    if (!FadeBorder)
    {
        FadeBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FadeBorder"));
        FadeBorder->SetBrushColor(FLinearColor::Black);
        FadeBorder->SetRenderOpacity(1.f);

        if (RootCanvas)
        {
            RootCanvas->AddChild(FadeBorder);
            if (UCanvasPanelSlot* InSlot = Cast<UCanvasPanelSlot>(FadeBorder->Slot))
            {
                InSlot->SetAnchors(FAnchors(0, 0, 1, 1));
                InSlot->SetOffsets(FMargin(0));
                InSlot->SetZOrder(1000);
            }
        }
    }

    PlayFadeOut(FadeBorder, 1.5f);

    // LogoImage 페이드인
    if (LogoImage)
    {
        FTimerHandle LogoFadeHandle;
        GetWorld()->GetTimerManager().SetTimer(LogoFadeHandle, [this]()
        {
            PlayFadeIn(LogoImage);
        }, 0.0f, false);
    }

    // MenuImage 페이드인 (LogoImage보다 0.5초 뒤)
    if (MenuImage)
    {
        FTimerHandle MenuFadeHandle;
        GetWorld()->GetTimerManager().SetTimer(MenuFadeHandle, [this]()
        {
            PlayFadeIn(MenuImage);
        }, 0.5f, false);
    }

    UpdateMenuSelection();
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 종료"));
}

// 검은 화면 페이드아웃 함수
void UTitleLevelWidget::PlayFadeOut(UBorder* TargetBorder, float Duration)
{
    if (!TargetBorder) return;

    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);

    FTimerHandle FadeHandle;
    GetWorld()->GetTimerManager().SetTimer(FadeHandle, [TargetBorder, Duration, TickInterval, Elapsed]()
    {
        *Elapsed += TickInterval;
        float Alpha = 1.f - FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
        TargetBorder->SetRenderOpacity(Alpha);

        if (Alpha <= 0.f)
        {
            TargetBorder->SetRenderOpacity(0.f);
            TargetBorder->RemoveFromParent();
            delete Elapsed;
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

    // FTimerHandle을 람다 외부에 선언하고, 포인터로 캡처
    FTimerHandle* FadeHandle = new FTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(*FadeHandle, [this, TargetImage, FadeDuration, TickInterval, Elapsed, FadeHandle]()
    {
        *Elapsed += TickInterval;
        float Alpha = FMath::Clamp(*Elapsed / FadeDuration, 0.f, 1.f);
        TargetImage->SetRenderOpacity(Alpha);

        if (Alpha >= 1.f)
        {
            GetWorld()->GetTimerManager().ClearTimer(*FadeHandle);
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