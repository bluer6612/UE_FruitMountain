#include "MainMenuWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuManager.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/UI/TitleLevel/Manager/MenuIndicatorAnimator.h"
#include "TimerManager.h"

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

void UMainMenuWidget::InitializeMainMenuWidget()
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
    }

    // 2. 페이드 아웃 재생
    if (!MainMenuFadeBorder)
    {
        UE_LOG(LogTemp, Error, TEXT("MainMenuFadeBorder가 블루프린트에 생성되지 않았습니다!"));
    }
    else
    {
        // 페이드 보더 초기화 (완전 불투명 검은색으로 시작)
        MainMenuFadeBorder->SetBrushColor(FLinearColor(0, 0, 0, 1));
        MainMenuFadeBorder->SetRenderOpacity(1.0f);
        
        if (UCanvasPanelSlot* BorderSlot = Cast<UCanvasPanelSlot>(MainMenuFadeBorder->Slot))
        {
            FVector2D ViewportSize = FVector2D(1920 * 3, 1080 * 2);
            BorderSlot->SetSize(ViewportSize);
            BorderSlot->SetPosition(FVector2D(-100, 0));
            BorderSlot->SetZOrder(20000);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MainMenuFadeBorder의 CanvasPanelSlot을 가져올 수 없음"));
        }

        PlayFadeOut(MainMenuFadeBorder);
    }
}

void UMainMenuWidget::StartLogoAndMenuFadeIn()
{
    // 1. 인디케이터 애니메이터 초기화
    if (!IndicatorAnimator)
    {
        IndicatorAnimator = NewObject<UMenuIndicatorAnimator>(this);
    }
    IndicatorAnimator->Initialize(SelectIndicator);

    // 2. 로고 이미지 페이드 인
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
        if (!WeakThis.IsValid())
        {
            return;
        }

        if (WeakThis->MenuImage)
        {
            WeakThis->PlayFadeIn(WeakThis->MenuImage);
            FTimerHandle IndicatorFadeHandle;
            UWorld* World = WeakThis->GetWorld();
            if (World)
            {
                World->GetTimerManager().SetTimer(IndicatorFadeHandle, [WeakThis]()
                {
                    if (WeakThis->SelectIndicator)
                    {
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