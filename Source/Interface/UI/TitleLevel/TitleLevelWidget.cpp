#include "TitleLevelWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

UTitleLevelWidget::UTitleLevelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
    FadeOutDuration = 2.5f;
    bHasScriptImplementedTick = true;
}

void UTitleLevelWidget::InitializeTitleWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 진입"));
    
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
        
        UE_LOG(LogTemp, Warning, TEXT("UI 요소 생성 완료 - Logo: %s, Menu: %s"), 
            IsValid(LogoImage) ? TEXT("유효") : TEXT("nullptr"),
            IsValid(MenuImage) ? TEXT("유효") : TEXT("nullptr"));
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
            
            UE_LOG(LogTemp, Warning, TEXT("FadeBorder 슬롯 설정 완료 - 크기: %s, ZOrder: %d"), 
                *BorderSlot->GetSize().ToString(), BorderSlot->GetZOrder());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FadeBorder의 CanvasPanelSlot을 가져올 수 없음"));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("블루프린트 FadeBorder 초기화 완료 - Opacity=%f"), 
               FadeBorder->GetRenderOpacity());
    }
    
    // 2. 페이드 아웃 시작
    if (FadeBorder)
    {
        PlayFadeOut();
        
        UE_LOG(LogTemp, Warning, TEXT("타이머 기반 FadeBorder 페이드아웃 시작"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FadeBorder 없음! 페이드아웃 불가"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 종료"));
}

void UTitleLevelWidget::StartLogoAndMenuFadeIn()
{
    if (LogoImage)
    {
        UE_LOG(LogTemp, Warning, TEXT("LogoImage 페이드인 시작"));
        PlayFadeIn(LogoImage);
    }
    
    FTimerHandle MenuFadeHandle;
    GetWorld()->GetTimerManager().SetTimer(MenuFadeHandle, [this]()
    {
        if (MenuImage)
        {
            UE_LOG(LogTemp, Warning, TEXT("MenuImage 페이드인 시작"));
            PlayFadeIn(MenuImage);
        }
    }, 0.5f, false);
}

// 타이머 기반 페이드아웃 함수 수정
void UTitleLevelWidget::PlayFadeOut()
{
    if (!FadeBorder)
    {
        return;
    }

    // PlayFadeIn과 동일한 구조로 변경
    const float FadeDuration = FadeOutDuration; // 클래스 멤버 변수 사용
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

        // 로깅 (10프레임마다)
        if (FMath::Fmod(*Elapsed, 0.2f) < TickInterval)
        {
            UE_LOG(LogTemp, Warning, TEXT("FadeOut 진행 중: %f초 / %f초, Alpha=%f"), 
                  *Elapsed, FadeDuration, Alpha);
        }

        if (*Elapsed >= FadeDuration)
        {
            WeakBorder->SetRenderOpacity(0.0f);
            UE_LOG(LogTemp, Warning, TEXT("FadeOut 완료: 소요 시간=%f초"), FadeDuration);
            
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
    
    UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut 시작: Duration=%f"), FadeDuration);
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