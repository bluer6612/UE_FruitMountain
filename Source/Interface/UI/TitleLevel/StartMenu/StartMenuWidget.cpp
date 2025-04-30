#include "StartMenuWidget.h"
#include "StartMenuManager.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "StartMenuWidget.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"
#include "Interface/HUD/FruitHUD.h"

UStartMenuWidget::UStartMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 생성자
    MenuManager = ObjectInitializer.CreateDefaultSubobject<UStartMenuManager>(this, TEXT("StartMenuManager"));
}

void UStartMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 키보드 포커스 설정
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

void UStartMenuWidget::InitializeMenuManager()
{
    // InitializeMenuManager()
    if (SelectIndicator && !MenuManager)
    {
        MenuManager = NewObject<UStartMenuManager>(this);
        if (MenuManager)
        {
            MenuManager->Initialize(this);
        }
    }
}

void UStartMenuWidget::InitializeStartMenu()
{
    UUIWidgetRenderer* Renderer = UUIWidgetRenderer::GetInstance();
    if (!Renderer)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeStartMenu: Renderer가 nullptr"));
        return;
    }

    // 게임 모드 메뉴 생성 (화면 중앙에 배치)
    GameModeMenuImage = Renderer->PrepareUIWidget(
        EWidgetImageType::UI_Title_GameModeMenu,
        TEXT("/Game/UI/TitleLevel/UI_Title_GameMode1"),
        FVector2D(1526.f, 828.f),
        0.f, 0.f);
        
    // nullptr 체크 추가
    if (GameModeMenuImage)
    {
        GameModeMenuImage->SetRenderOpacity(0.f);
        
        // Z-Order 설정
        if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(GameModeMenuImage->Slot))
        {
            MenuSlot->SetZOrder(5);
            
            // 중앙 배치를 위한 앵커 및 정렬 설정
            MenuSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
            MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            MenuSlot->SetPosition(FVector2D(0.f, 0.f)); // 중앙 기준점
        }
        
        // 페이드 인 효과
        PlayFadeIn(GameModeMenuImage);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeStartMenu: GameModeMenuImage 생성 실패"));
    }

    // 선택 인디케이터 (메뉴 왼쪽에 배치)
    SelectIndicator = Renderer->PrepareUIWidget(
        EWidgetImageType::UI_Title_Select,
        TEXT("/Game/UI/TitleLevel/UI_Title_Select"),
        FVector2D(59.f, 59.f), 
        -230.f, -30.f); // 왼쪽으로 오프셋 (메뉴 왼쪽에 위치)
        
    // nullptr 체크 추가
    if (SelectIndicator)
    {
        SelectIndicator->SetRenderOpacity(0.f);
        
        // Z-Order 설정
        if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
        {
            IndicatorSlot->SetZOrder(10);
            
            // 왼쪽 배치를 위한 조정
            IndicatorSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
            IndicatorSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        }
        
        // 페이드 인 효과
        PlayFadeIn(SelectIndicator);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeStartMenu: SelectIndicator 생성 실패"));
        return; // 선택 인디케이터가 없으면 메뉴 관리자 초기화 불가
    }

    // 메뉴 관리자 초기화
    if (MenuManager)
    {
        MenuManager->Initialize(this);
        MenuManager->UpdateMenuSelection();
    }
}

void UStartMenuWidget::PlayFadeIn(UImage* TargetImage)
{
    if (!TargetImage)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayFadeIn: 대상 이미지가 nullptr"));
        return;
    }

    const float FadeDuration = FadeInDuration;
    const float TickInterval = 0.02f;
    float* Elapsed = new float(0.f);
    FTimerHandle* FadeHandle = new FTimerHandle;

    TWeakObjectPtr<UStartMenuWidget> WeakThis(this);
    TWeakObjectPtr<UImage> WeakImage(TargetImage);

    GetWorld()->GetTimerManager().SetTimer(*FadeHandle, [WeakThis, WeakImage, FadeDuration, TickInterval, Elapsed, FadeHandle]()
    {
        if (!WeakThis.IsValid() || !WeakImage.IsValid())
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
        float Alpha = FMath::Clamp(*Elapsed / FadeDuration, 0.f, 1.f);
        WeakImage->SetRenderOpacity(Alpha);

        if (Alpha >= 1.f)
        {
            if (UWorld* World = WeakThis->GetWorld())
            {
                World->GetTimerManager().ClearTimer(*FadeHandle);
            }
            delete FadeHandle;
            delete Elapsed;
        }
    }, TickInterval, true);
}

FReply UStartMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
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

void UStartMenuWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    Super::NativeDestruct();
}

void UStartMenuWidget::StartGame(int32 GameMode)
{
    // 모든 위젯 숨김
    if (GameModeMenuImage)
    {
        GameModeMenuImage->SetVisibility(ESlateVisibility::Hidden);
        GameModeMenuImage->SetRenderOpacity(0.f);
    }
    if (SelectIndicator)
    {
        SelectIndicator->SetVisibility(ESlateVisibility::Hidden);
        SelectIndicator->SetRenderOpacity(0.f);
    }

    // 타이머 모두 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }

    // 게임 모드에 따른 설정 (나중에 구현)
    // 여기서 GameMode 값에 따라 다른 설정을 적용할 수 있음

    // 레벨 전환 예약
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
    
    // 위젯 제거
    RemoveFromParent();
}

void UStartMenuWidget::BackToMainMenu()
{
    // 타이머 모두 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }

    // 위젯을 뷰포트에서 제거
    RemoveFromParent();

    // 타이틀 메뉴 위젯 다시 표시
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        // MainMenuWidget 찾기 (앱에 맞게 수정 필요)
        AFruitHUD* FruitHUD = Cast<AFruitHUD>(PC->GetHUD());
        if (FruitHUD && FruitHUD->GetMainMenuWidget()) {
            UMainMenuWidget* TitleWidget = FruitHUD->GetMainMenuWidget();
            // 메뉴와 로고 다시 표시
            if (TitleWidget->MenuImage)
            {
                TitleWidget->MenuImage->SetVisibility(ESlateVisibility::Visible);
                TitleWidget->MenuImage->SetRenderOpacity(1.0f);
            }
            if (TitleWidget->LogoImage)
            {
                TitleWidget->LogoImage->SetVisibility(ESlateVisibility::Visible);
                TitleWidget->LogoImage->SetRenderOpacity(1.0f);
            }
            
            // 타이틀 메뉴의 선택기도 다시 표시
            if (TitleWidget->SelectIndicator)
            {
                TitleWidget->SelectIndicator->SetVisibility(ESlateVisibility::Visible);
                TitleWidget->SelectIndicator->SetRenderOpacity(1.0f);
            }
            
            // 포커스도 돌려줌
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(TitleWidget->TakeWidget());
            PC->SetInputMode(InputMode);
        }
    }
}