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

// 타이머 기반 페이드아웃 함수 추가
void UTitleLevelWidget::PlayFadeOut()
{
    if (!IsValid(FadeBorder))
    {
        UE_LOG(LogTemp, Error, TEXT("PlayFadeOut: FadeBorder가 유효하지 않음"));
        return;
    }
    
    // 초기 상태 설정
    FadeBorder->SetRenderOpacity(1.0f);
    UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut: 시작 - Duration=%f"), FadeOutDuration);
    
    // 페이드아웃을 위한 구조체 생성
    struct FFadeOutContext
    {
        float ElapsedTime = 0.0f;
        float Duration = 0.0f;
        FTimerHandle TimerHandle;
    };
    
    // 컨텍스트 생성
    FFadeOutContext* Context = new FFadeOutContext();
    Context->Duration = FadeOutDuration;
    
    // 약한 참조로 안전하게 처리
    TWeakObjectPtr<UTitleLevelWidget> WeakThis(this);
    TWeakObjectPtr<UBorder> WeakBorder(FadeBorder);
    
    // 더 자주 업데이트되도록 간격 조정 (약 60fps)
    float TickInterval = 0.016f;
    
    GetWorld()->GetTimerManager().SetTimer(
        Context->TimerHandle,
        [WeakThis, WeakBorder, Context, TickInterval]()
        {
            if (!WeakThis.IsValid() || !WeakBorder.IsValid())
            {
                UE_LOG(LogTemp, Warning, TEXT("PlayFadeOut: 위젯이 더 이상 유효하지 않음"));
                if (UWorld* World = GEngine->GetWorld())
                {
                    World->GetTimerManager().ClearTimer(Context->TimerHandle);
                }
                delete Context;
                return;
            }
            
            // 시간 업데이트
            Context->ElapsedTime += TickInterval;
            
            // 알파값 계산 (1.0에서 0.0으로)
            float Alpha = 1.0f - FMath::Clamp(Context->ElapsedTime / Context->Duration, 0.0f, 1.0f);
            WeakBorder->SetRenderOpacity(Alpha);
            
            // 완료되면 정리
            if (Context->ElapsedTime >= Context->Duration)
            {
                WeakBorder->SetRenderOpacity(0.0f);
                UE_LOG(LogTemp, Warning, TEXT("FadeOut 완료: 소요 시간=%f초"), Context->Duration);
                
                // 타이머 정리
                WeakThis->GetWorld()->GetTimerManager().ClearTimer(Context->TimerHandle);
                
                // 로고와 메뉴 페이드인 시작
                WeakThis->StartLogoAndMenuFadeIn();
                
                delete Context;
            }
        },
        TickInterval, true);
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