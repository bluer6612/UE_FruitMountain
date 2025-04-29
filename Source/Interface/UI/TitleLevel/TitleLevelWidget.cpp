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
        
        // 선택 표시기 생성 부분 수정
        SelectIndicator = Renderer->PrepareUIWidget(EWidgetImageType::UI_Title_Select,
            TEXT("/Game/UI/TitleLevel/UI_Title_Select"),
            FVector2D(50.f, 50.f), 150.f - 75.f, MenuPositions[0]);  // 메뉴 X 위치(150.f)에서 75.f만큼 왼쪽에 위치
        SelectIndicator->SetRenderOpacity(0.f);  // 페이드인 때까지 숨김
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
                    UpdateMenuSelection();  // 초기 선택 상태 설정
                }
            }, 0.2f, false);
        }
    }, 0.5f, false);
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

void UTitleLevelWidget::UpdateMenuSelection()
{
    if (!SelectIndicator)
    {
        UE_LOG(LogTemp, Error, TEXT("메뉴 선택 표시기가 유효하지 않습니다."));
        return;
    }
    
    // 현재 선택된 메뉴 항목에 표시기 위치 설정
    if (UCanvasPanelSlot* IndicatorSlot = Cast<UCanvasPanelSlot>(SelectIndicator->Slot))
    {
        // X 위치는 메뉴 위치(150.f)에서 75.f 뺀 값으로 고정
        float IndicatorX = 150.f - 75.f;
        float TargetY = MenuPositions[CurrentMenuIndex];
        
        // 부드러운 이동 애니메이션
        FVector2D NewPos(IndicatorX, TargetY);
        IndicatorSlot->SetPosition(NewPos);
        
        // 활성화된 메뉴 시각적 피드백
        IndicatorSlot->SetSize(FVector2D(60.f, 60.f));
        
        // 애니메이션 효과
        PlaySelectionAnimation();
    }
}

// 선택 효과를 위한 간단한 애니메이션
void UTitleLevelWidget::PlaySelectionAnimation()
{
    if (!SelectIndicator) return;
    
    // 깜박임 또는 크기 변화 등의 간단한 애니메이션
    SelectIndicator->SetRenderScale(FVector2D(1.2f, 1.2f));
    
    FTimerHandle ResetHandle;
    GetWorld()->GetTimerManager().SetTimer(ResetHandle, [this]()
    {
        if (SelectIndicator)
        {
            SelectIndicator->SetRenderScale(FVector2D(1.0f, 1.0f));
        }
    }, 0.15f, false);
}

FReply UTitleLevelWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    
    // 위로 이동 (UP, W)
    if (Key == EKeys::Up || Key == EKeys::W)
    {
        // 순환식으로 인덱스 감소
        CurrentMenuIndex = (CurrentMenuIndex - 1 + MenuItemCount) % MenuItemCount;
        UpdateMenuSelection();
        return FReply::Handled();
    }
    
    // 아래로 이동 (DOWN, S)
    if (Key == EKeys::Down || Key == EKeys::S)
    {
        // 순환식으로 인덱스 증가
        CurrentMenuIndex = (CurrentMenuIndex + 1) % MenuItemCount;
        UpdateMenuSelection();
        return FReply::Handled();
    }
    
    // 선택 (Enter, SpaceBar)
    if (Key == EKeys::SpaceBar || Key == EKeys::Enter)
    {
        OnMenuSelect();
        return FReply::Handled();
    }
    
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTitleLevelWidget::OnMenuSelect()
{
    switch (CurrentMenuIndex)
    {
        case 0: // 게임 시작
            UGameplayStatics::OpenLevel(this, TEXT("PlayLevel"));
            break;
            
        case 1: // 랭킹
            // 랭킹 화면 표시
            OpenRankingMenu();
            break;
            
        case 2: // 옵션
            // 옵션 메뉴 표시
            OpenOptionsMenu();
            break;
            
        case 3: // 크레딧
            // 크레딧 표시
            OpenCreditScreen();
            break;
    }
}

// 새로운 메뉴 함수들 추가
void UTitleLevelWidget::OpenRankingMenu()
{
    // 랭킹 화면 표시 로직
    // 예: 랭킹 위젯 생성 및 표시
    UE_LOG(LogTemp, Warning, TEXT("랭킹 메뉴 열기"));
}

void UTitleLevelWidget::OpenOptionsMenu()
{
    // 옵션 메뉴 표시 로직
    // 예: 옵션 위젯 생성 및 표시
    UE_LOG(LogTemp, Warning, TEXT("옵션 메뉴 열기"));
}

void UTitleLevelWidget::OpenCreditScreen()
{
    // 크레딧 화면 표시 로직
    // 예: 크레딧 위젯 생성 및 표시
    UE_LOG(LogTemp, Warning, TEXT("크레딧 화면 열기"));
}