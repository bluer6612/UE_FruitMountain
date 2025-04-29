#include "TitleLevelWidget.h"
#include "TitleMenuManager.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "TimerManager.h"

UTitleLevelWidget::UTitleLevelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
    bHasScriptImplementedTick = true;

    // 메뉴 관리자를 생성자에서 올바르게 생성
    MenuManager = ObjectInitializer.CreateDefaultSubobject<UTitleMenuManager>(this, TEXT("MenuManager"));
}

void UTitleLevelWidget::NativeDestruct()
{
    Super::NativeDestruct();
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void UTitleLevelWidget::InitializeTitleWidget()
{
    UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] InitializeTitleWidget 시작"));
    
    // 1. 게임 UI 요소 생성
    UUIWidgetRenderer* Renderer = UUIWidgetRenderer::GetInstance();
    if (Renderer)
    {
        UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] UIWidgetRenderer 인스턴스 획득 성공"));
        
        LogoImage = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Logo,
            TEXT("/Game/UI/TitleLevel/UI_Title_Logo"),
            FVector2D(633.f, 369.f), 150.f, 270.f);
        LogoImage->SetRenderOpacity(0.f);
        UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] LogoImage 생성: %s"), IsValid(LogoImage) ? TEXT("성공") : TEXT("실패"));
        
        MenuImage = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Menu,
            TEXT("/Game/UI/TitleLevel/UI_Title_Menu"),
            FVector2D(592.f, 359.f), 150.f, 50.f);
        MenuImage->SetRenderOpacity(0.f);
        UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] MenuImage 생성: %s"), IsValid(MenuImage) ? TEXT("성공") : TEXT("실패"));
        
        SelectIndicator = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Select,
            TEXT("/Game/UI/TitleLevel/UI_Title_Select"),
            FVector2D(59.f, 59.f), 150.f - 75.f, MenuPositions[0]);
        SelectIndicator->SetRenderOpacity(0.f);
        UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] SelectIndicator 생성: %s (위치: X=%f, Y=%f)"), 
               IsValid(SelectIndicator) ? TEXT("성공") : TEXT("실패"), 
               150.f - 75.f, MenuPositions[0]);

        // 메뉴 Z-Order 설정
        if (UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(MenuImage->Slot))
        {
            MenuSlot->SetZOrder(5); // 메뉴는 5로 설정
            UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] MenuImage Z-Order 설정: 5"));
        }
        
        // 로고 Z-Order 설정
        if (UCanvasPanelSlot* LogoSlot = Cast<UCanvasPanelSlot>(LogoImage->Slot))
        {
            LogoSlot->SetZOrder(1); // 로고는 맨 뒤
            UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] LogoImage Z-Order 설정: 1"));
        }

        // 선택 표시기 Z-Order 설정
        if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
        {
            IndicatorSlot->SetZOrder(10); // 메뉴보다 높은 Z-Order 설정
            UE_LOG(LogTemp, Log, TEXT("[TitleLevelWidget] SelectIndicator Z-Order 설정: 10"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UIWidgetRenderer 인스턴스를 가져올 수 없음"));
    }

    // 바인딩된 FadeBorder가 있는지 확인
    if (!FadeBorder)
    {
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
    }
    
    // 2. 페이드 아웃 시작
    if (FadeBorder)
    {
        PlayFadeOut();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FadeBorder 없음! 페이드아웃 불가"));
    }
    
    // UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 종료"));
}

void UTitleLevelWidget::StartLogoAndMenuFadeIn()
{
    UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] StartLogoAndMenuFadeIn() 시작"));
    
    if (LogoImage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] LogoImage 페이드인 시작 (유효함)"));
        PlayFadeIn(LogoImage);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[TitleLevelWidget] LogoImage가 유효하지 않음 - 페이드인 실패"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] MenuImage 페이드인 타이머 설정 (0.5초 후)"));
    FTimerHandle MenuFadeHandle;
    GetWorld()->GetTimerManager().SetTimer(MenuFadeHandle, [this]()
    {
        UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] MenuImage 페이드인 타이머 콜백 실행"));
        
        if (MenuImage)
        {
            UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] MenuImage 페이드인 시작 (유효함)"));
            PlayFadeIn(MenuImage);
            
            // 메뉴가 표시된 후 선택 표시기 페이드인
            UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] SelectIndicator 페이드인 타이머 설정 (0.2초 후)"));
            FTimerHandle IndicatorFadeHandle;
            GetWorld()->GetTimerManager().SetTimer(IndicatorFadeHandle, [this]()
            {
                UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] SelectIndicator 페이드인 타이머 콜백 실행"));
                
                if (SelectIndicator)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] SelectIndicator 페이드인 시작 (유효함)"));
                    UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] SelectIndicator 위치: X=%f, Y=%f"),
                        Cast<UCanvasPanelSlot>(SelectIndicator->Slot) ? 
                        Cast<UCanvasPanelSlot>(SelectIndicator->Slot)->GetPosition().X : -1.f,
                        Cast<UCanvasPanelSlot>(SelectIndicator->Slot) ? 
                        Cast<UCanvasPanelSlot>(SelectIndicator->Slot)->GetPosition().Y : -1.f);
                        
                    PlayFadeIn(SelectIndicator);
                    
                    // 메뉴 관리자 초기화 및 첫번째 항목 선택
                    if (MenuManager)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] MenuManager 초기화 및 첫 항목 선택 (MenuManager 유효함)"));
                        MenuManager->Initialize(SelectIndicator, this);
                        MenuManager->UpdateMenuSelection();
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("[TitleLevelWidget] MenuManager가 유효하지 않음 - 초기화 실패"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("[TitleLevelWidget] SelectIndicator가 유효하지 않음 - 페이드인 실패"));
                }
                
                UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] SelectIndicator 페이드인 타이머 콜백 종료"));
            }, 0.2f, false);
            
            UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] SelectIndicator 페이드인 타이머 설정 완료"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[TitleLevelWidget] MenuImage가 유효하지 않음 - 페이드인 실패"));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] MenuImage 페이드인 타이머 콜백 종료"));
    }, 0.5f, false);
    
    UE_LOG(LogTemp, Warning, TEXT("[TitleLevelWidget] StartLogoAndMenuFadeIn() 종료: 타이머 설정 완료"));
}

void UTitleLevelWidget::PlayFadeOut()
{
    if (!FadeBorder)
    {
        return;
    }
    
    // UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut 시작: Duration=%f"), FadeDuration);

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
    // MenuManager에게 키 처리 위임
    if (MenuManager && MenuManager->HandleKeyDown(InKeyEvent.GetKey()))
    {
        return FReply::Handled();
    }
    
    // MenuManager에서 처리되지 않은 키는 기본 처리
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}