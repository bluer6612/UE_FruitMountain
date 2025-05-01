#include "StartMenuWidget.h"
#include "StartMenuManager.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Interface/HUD/FruitHUD.h"
#include "Interface/UI/TitleLevel/Manager/MenuIndicatorAnimator.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"

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
    StartMenuImage = Renderer->PrepareUIWidget(
        EWidgetImageType::UI_Title_GameModeMenu,
        TEXT("/Game/UI/TitleLevel/UI_Title_GameMode1"),
        FVector2D(1526.f, 828.f),
        0.f, 0.f);
        
    // nullptr 체크 추가
    if (StartMenuImage)
    {
        StartMenuImage->SetRenderOpacity(0.f);
        
        // Z-Order 설정
        if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(StartMenuImage->Slot))
        {
            MenuSlot->SetZOrder(5);
            
            // 중앙 배치를 위한 앵커 및 정렬 설정
            MenuSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
            MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            MenuSlot->SetPosition(FVector2D(0.f, 0.f)); // 중앙 기준점
        }
        
        // 페이드 인 효과
        PlayFadeIn(StartMenuImage);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeStartMenu: StartMenuImage 생성 실패"));
    }

    // 선택 인디케이터 생성
    SelectIndicator = Renderer->PrepareUIWidget(
        EWidgetImageType::UI_Title_Select,
        TEXT("/Game/UI/TitleLevel/UI_Title_Select"),
        FVector2D(59.f, 59.f), 
        -230.f, -30.f);

    if (SelectIndicator)
    {
        SelectIndicator->SetRenderOpacity(0.f);
        if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
        {
            IndicatorSlot->SetZOrder(10);
            IndicatorSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
            IndicatorSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        }
        PlayFadeIn(SelectIndicator);

        // 반드시 여기서 IndicatorAnimator를 생성/초기화
        if (!IndicatorAnimator)
        {
            IndicatorAnimator = NewObject<UMenuIndicatorAnimator>(this);
        }
        IndicatorAnimator->Initialize(SelectIndicator);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeStartMenu: SelectIndicator 생성 실패"));
        return;
    }

    // 메뉴 관리자 초기화
    if (MenuManager)
    {
        MenuManager->Initialize(this);
        MenuManager->UpdateMenuSelection();
    }
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

void UStartMenuWidget::BackToMainMenu()
{
    // 타이머 모두 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }

    if (StartMenuImage)
    {
        StartMenuImage->SetRenderOpacity(0.f);
    }
    if (SelectIndicator)
    {
        SelectIndicator->SetRenderOpacity(0.f);
    }

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