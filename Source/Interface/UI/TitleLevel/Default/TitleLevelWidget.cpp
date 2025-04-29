#include "TitleLevelWidget.h"
#include "TitleMenuManager.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/HUD/FruitHUD.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UTitleLevelWidget::UTitleLevelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 메뉴 관리자를 생성자에서 올바르게 생성
    MenuManager = ObjectInitializer.CreateDefaultSubobject<UTitleMenuManager>(this, TEXT("MenuManager"));
}

void UTitleLevelWidget::NativeConstruct()
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

void UTitleLevelWidget::InitializeMenuManager()
{
    if (SelectIndicator)
    {
        // 메뉴 관리자 생성 및 초기화
        MenuManager = NewObject<UTitleMenuManager>(this);
        if (MenuManager)
        {
            MenuManager->Initialize(SelectIndicator, this);
        }
    }
}

void UTitleLevelWidget::InitializeTitleWidget()
{
    // 1. 게임 UI 요소 생성
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

        PlayFadeOut();
    }
}
    

void UTitleLevelWidget::StartLogoAndMenuFadeIn()
{
    if (LogoImage)
    {
        PlayFadeIn(LogoImage);
    }
    
    FTimerHandle MenuFadeHandle;
    GetWorld()->GetTimerManager().SetTimer(MenuFadeHandle, [this]()
    {
        if (MenuImage)
        {
            PlayFadeIn(MenuImage);
            
            // 메뉴가 표시된 후 선택 표시기 페이드인
            FTimerHandle IndicatorFadeHandle;
            GetWorld()->GetTimerManager().SetTimer(IndicatorFadeHandle, [this]()
            {
                if (SelectIndicator)
                {
                    PlayFadeIn(SelectIndicator);
                    
                    // 메뉴 관리자 초기화 및 첫번째 항목 선택
                    if (MenuManager)
                    {
                        MenuManager->Initialize(SelectIndicator, this);
                        MenuManager->UpdateMenuSelection();
                    }
                }
            }, 0.25f, false);
        }
    }, 0.5f, false);
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
    GetWorld()->GetTimerManager().SetTimer(*FadeHandle, [WeakThis, WeakBorder, FadeDuration, TickInterval, Elapsed, FadeHandle]()
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
        
        // 알파값 계산 (1.0에서 0.0으로 감소)
        float Alpha = 1.0f - FMath::Clamp(*Elapsed / FadeDuration, 0.f, 1.f);
        WeakBorder->SetRenderOpacity(Alpha);

        if (*Elapsed >= FadeDuration)
        {
            WeakBorder->SetRenderOpacity(0.0f);
            
            // 타이머 정리
            if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()))
            {
                World->GetTimerManager().ClearTimer(*FadeHandle);
            }
            
            // 로고와 메뉴 페이드인 시작
            WeakThis->StartLogoAndMenuFadeIn();
            
            delete FadeHandle;
            delete Elapsed;
        }
    }, TickInterval, true);
}

void UTitleLevelWidget::PlayFadeIn(UImage* TargetImage)
{
    if (!TargetImage)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayFadeIn: 대상 이미지가 nullptr"));
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
                if (UWorld* World = GEngine && WeakThis.IsValid() ? GEngine->GetWorldFromContextObjectChecked(WeakThis.Get()) : nullptr)
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

void UTitleLevelWidget::NativeDestruct()
{
    // 메뉴 관리자의 애니메이션 중지
    if (MenuManager)
    {
        // 필요한 경우 메뉴 관리자의 타이머 정리 등 수행
        MenuManager->StartIndicatorAnimation(false);
    }
    
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        // 혹시 남아있는 타이머가 있다면 정리
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    // 부모 클래스의 NativeDestruct 호출
    Super::NativeDestruct();
}

void UTitleLevelWidget::StartGame()
{
    // 1. 모든 입력 비활성화
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->DisableInput(PC);
        
        // 2. HUD 정리 (옵셔널)
        if (AFruitHUD* FruitHUD = Cast<AFruitHUD>(PC->GetHUD()))
        {
            FruitHUD->ClearTitleWidget();
        }
    }

    // 3. 모든 애니메이션 중지
    if (SelectIndicator)
    {
        SelectIndicator->SetRenderOpacity(0.0f);
    }
    
    // 4. 직접 레벨 로드 요청 (메뉴매니저가 아닌 위젯에서 직접 수행)
    if (UWorld* World = GetWorld())
    {
        // 약간의 지연 후 레벨 전환
        FTimerHandle GameStartHandle;
        World->GetTimerManager().SetTimer(
            GameStartHandle,
            FTimerDelegate::CreateLambda([World]()
            {
                UGameplayStatics::OpenLevel(World, TEXT("PlayLevel"));
            }),
            0.2f,
            false
        );
    }
}