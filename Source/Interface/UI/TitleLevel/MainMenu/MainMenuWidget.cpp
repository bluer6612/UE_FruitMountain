#include "MainMenuWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuManager.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/UI/TitleLevel/Manager/MenuIndicatorAnimator.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
    // 메뉴 관리자를 생성자에서 올바르게 생성
    MenuManager = ObjectInitializer.CreateDefaultSubobject<UMainMenuManager>(this, TEXT("MenuManager"));
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 메뉴 관리자 초기화
    InitializeMenuManager();
}

void UMainMenuWidget::InitializeMenuManager()
{
    if (SelectIndicator && !MenuManager)
    {
        MenuManager = NewObject<UMainMenuManager>(this);
        if (MenuManager)
        {
            MenuManager->Initialize(this);
        }
    }
}

void UMainMenuWidget::InitializeTitleWidget()
{
    UUIWidgetRenderer* Renderer = UUIWidgetRenderer::GetInstance();

    if (Renderer)
    {
        LogoImage = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Logo,
            TEXT("/Game/UI/TitleLevel/UI_Title_Logo"),
            FVector2D(633.f, 369.f), 150.f, 270.f);
        LogoImage->SetRenderOpacity(0.f);

        MenuImage = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Menu,
            TEXT("/Game/UI/TitleLevel/UI_Title_Menu"),
            FVector2D(592.f, 359.f), 150.f, 50.f);
        MenuImage->SetRenderOpacity(0.f);

        SelectIndicator = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Select,
            TEXT("/Game/UI/TitleLevel/UI_Title_Select"),
            FVector2D(59.f, 59.f), 0.f, 0.f);
        SelectIndicator->SetRenderOpacity(0.f);

        if (!IndicatorAnimator)
        {
            IndicatorAnimator = NewObject<UMenuIndicatorAnimator>(this);
        }
        IndicatorAnimator->Initialize(SelectIndicator); // 항상 SelectIndicator를 넘김
        UE_LOG(LogTemp, Warning, TEXT("IndicatorAnimator가 초기화 됨. SelectIndicator=%p, Animator->Indicator=%p"), SelectIndicator, IndicatorAnimator->GetIndicator());
    }

    // 2. 페이드 아웃 재생
    if (!FadeBorder)
    {
        // 중요 오류 로그 유지
        UE_LOG(LogTemp, Error, TEXT("FadeBorder가 블루프린트에 생성되지 않았습니다!"));
    }
    else
    {
        // 페이드 보더 초기화 (완전 불투명 검은색으로 시작)
        FadeBorder->SetBrushColor(FLinearColor(0, 0, 0, 1));
        FadeBorder->SetRenderOpacity(1.0f);
        
        if (UCanvasPanelSlot* BorderSlot = Cast<UCanvasPanelSlot>(FadeBorder->Slot))
        {
            FVector2D ViewportSize = FVector2D(1920 * 3, 1080 * 2);
            BorderSlot->SetSize(ViewportSize);
            BorderSlot->SetPosition(FVector2D(-100, 0));
            BorderSlot->SetZOrder(20000);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FadeBorder의 CanvasPanelSlot을 가져올 수 없음"));
        }

        UTitleLevelWidget::PlayFadeOut();
    }
}

void UMainMenuWidget::StartLogoAndMenuFadeIn()
{
    if (LogoImage)
    {
        PlayFadeIn(LogoImage);
    }
    
    // 입력 활성화
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    FTimerHandle MenuFadeHandle;
    TWeakObjectPtr<UMainMenuWidget> WeakThis(this);
    GetWorld()->GetTimerManager().SetTimer(MenuFadeHandle, [WeakThis]()
    {
        if (!WeakThis.IsValid()) return;
        if (WeakThis->MenuImage)
        {
            WeakThis->PlayFadeIn(WeakThis->MenuImage);
            FTimerHandle IndicatorFadeHandle;
            UWorld* World = WeakThis->GetWorld();
            if (World)
            {
                World->GetTimerManager().SetTimer(IndicatorFadeHandle, [WeakThis]()
                {
                    if (!WeakThis.IsValid()) return;
                    if (WeakThis->SelectIndicator)
                    {
                        //WeakThis->PlayFadeIn(WeakThis->SelectIndicator);
                        if (WeakThis->MenuManager)
                        {
                            WeakThis->MenuManager->Initialize(WeakThis.Get());
                            WeakThis->MenuManager->UpdateMenuSelection();
                        }
                    }
                }, 0.25f, false);
            }
        }
    }, 0.5f, false);
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (MenuManager)
    {
        bool bHandled = MenuManager->HandleKeyDown(InKeyEvent.GetKey());
        if (bHandled)
        {
            return FReply::Handled();
        }
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMainMenuWidget::NativeDestruct()
{
    UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget::NativeDestruct 호출") );
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    Super::NativeDestruct();
}

void UMainMenuWidget::StartGame()
{
    // 1. 모든 타이틀 위젯 숨김
    if (LogoImage)
    {
        LogoImage->SetVisibility(ESlateVisibility::Hidden);
        LogoImage->SetRenderOpacity(0.f);
    }
    if (MenuImage)
    {
        MenuImage->SetVisibility(ESlateVisibility::Hidden);
        MenuImage->SetRenderOpacity(0.f);
    }
    if (SelectIndicator)
    {
        SelectIndicator->SetVisibility(ESlateVisibility::Hidden);
        SelectIndicator->SetRenderOpacity(0.f);
    }

    // 2. 타이머 모두 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }

    // 3. 페이드 아웃 효과
    if (FadeBorder)
    {
        UTitleLevelWidget::PlayFadeOut();
    }

    // 4. 레벨 전환 예약
    if (UWorld* World = GetWorld())
    {
        FTimerHandle GameStartHandle;
        TWeakObjectPtr<UWorld> WeakWorld(World);
        World->GetTimerManager().SetTimer(
            GameStartHandle,
            FTimerDelegate::CreateLambda([WeakWorld]()
            {
                if (WeakWorld.IsValid())
                {
                    UGameplayStatics::OpenLevel(WeakWorld.Get(), TEXT("PlayLevel"));
                }
            }),
            0.2f,
            false
        );
    }
}