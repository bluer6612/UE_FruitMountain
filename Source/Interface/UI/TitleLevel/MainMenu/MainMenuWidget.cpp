#include "MainMenuWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuManager.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 메뉴 관리자를 생성자에서 올바르게 생성
    MenuManager = ObjectInitializer.CreateDefaultSubobject<UMainMenuManager>(this, TEXT("MenuManager"));
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 키보드 포커스 설정 - 이걸 해야 키 입력을 받을 수 있음
    bIsFocusable = true;
    
    // UI가 생성될 때 자동으로 포커스 가져오기
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    
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
            MenuManager->Initialize(SelectIndicator, this);
        }
    }
}

void UMainMenuWidget::InitializeTitleWidget()
{
    UUIWidgetRenderer* Renderer = UUIWidgetRenderer::GetInstance();

    // 1. 게임 UI 요소 생성
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

        // 메뉴 Z-Order 설정
        if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(MenuImage->Slot))
        {
            MenuSlot->SetZOrder(5);
        }
        
        // 로고 Z-Order 설정
        if (UCanvasPanelSlot* LogoSlot = Cast<UCanvasPanelSlot>(LogoImage->Slot))
        {
            LogoSlot->SetZOrder(1);
        }

        // 선택 표시기 Z-Order 설정
        if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
        {
            IndicatorSlot->SetZOrder(10);
        }
    }

    // FadeBorder 및 페이드 관련 코드 완전 제거
}

FReply UMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // MenuManager가 있으면 키 이벤트 전달
    if (MenuManager)
    {
        bool bHandled = MenuManager->HandleKeyDown(InKeyEvent.GetKey());
        if (bHandled)
        {
            return FReply::Handled();
        }
    }
    
    // 처리되지 않은 키는 부모 클래스로 전달
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

    // 3. 레벨 전환 예약
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