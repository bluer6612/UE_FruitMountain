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
#include "Interface/HUD/FruitHUD.h"

UTitleLevelWidget::UTitleLevelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
    FadeOutDuration = 1.5f;
    bHasScriptImplementedTick = true;
}

// 위젯이 실제로 생성될 때 호출됨
void UTitleLevelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 간단한 지연 후 초기화 진행 (HUD가 생성될 시간 확보)
    FTimerHandle InitHandle;
    GetWorld()->GetTimerManager().SetTimer(InitHandle, this, &UTitleLevelWidget::InitializeTitleWidget, 0.2f, false);
}

void UTitleLevelWidget::InitializeTitleWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 진입"));
    
    // 1. HUD 확인
    AFruitHUD* FruitHUD = Cast<AFruitHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
    if (!FruitHUD)
    {
        UE_LOG(LogTemp, Error, TEXT("HUD가 아직 생성되지 않았습니다. 초기화 지연"));
        
        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(RetryHandle, this, &UTitleLevelWidget::InitializeTitleWidget, 0.1f, false);
        return;
    }
    
    // 2. 간단한 검은색 보더 생성 부분 수정
    if (!FadeBorder)
    {
        // 보더를 직접 생성하고 루트 캔버스에 추가
        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
        if (!RootCanvas)
        {
            // 루트 캔버스가 없으면 생성
            RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
            WidgetTree->RootWidget = RootCanvas;
        }
        
        // 간단히 보더 생성
        FadeBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        
        // 중요: 보더 내용물 설정
        FadeBorder->SetContent(WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass()));
        
        // 전체 불투명 검은색으로 설정
        FadeBorder->SetBrushColor(FLinearColor(0, 0, 0, 1));
        FadeBorder->SetRenderOpacity(1.0f);
        
        // 루트 캔버스에 보더 추가
        RootCanvas->AddChild(FadeBorder);
        
        // 전체 화면 크기로 설정
        if (UCanvasPanelSlot* BorderSlot = Cast<UCanvasPanelSlot>(FadeBorder->Slot))
        {
            BorderSlot->SetAnchors(FAnchors(0, 0, 1, 1));
            BorderSlot->SetOffsets(FMargin(0, 0, 0, 0));
            BorderSlot->SetZOrder(100000);  // ZOrder를 더 높게 설정
            BorderSlot->SetSize(FVector2D(3840, 2160));  // 명시적인 크기 설정
        }
        
        // 로그 및 디버깅
        UE_LOG(LogTemp, Warning, TEXT("페이드 보더 생성 완료 - Color=%s, Opacity=%f, ZOrder=100000"), 
            *FadeBorder->GetBrushColor().ToString(), FadeBorder->GetRenderOpacity());
    }
    
    // 3. 게임 UI 요소 생성
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
    }
    
    if (!LogoImage || !MenuImage)
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 위젯 생성 실패! Logo: %s, Menu: %s"), 
            LogoImage ? TEXT("유효") : TEXT("nullptr"),
            MenuImage ? TEXT("유효") : TEXT("nullptr"));
    }
    
    // 4. 페이드 아웃 시작
    if (FadeBorder)
    {
        FadeBorder->SetRenderOpacity(1.0f);
        
        // 페이드 변수 초기화
        FadeTime = 0.0f;
        FadeOutDuration = 1.5f;
        bIsFading = true;
        
        UE_LOG(LogTemp, Warning, TEXT("독립 페이드 스크린 페이드아웃 시작"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FadeBorder 없음! 페이드아웃 불가"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 종료"));
}

void UTitleLevelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // 페이드 중이라면 매 프레임 업데이트
    if (bIsFading && FadeBorder)
    {
        FadeTime += InDeltaTime;
        float Alpha = 1.0f - FMath::Clamp(FadeTime / FadeOutDuration, 0.0f, 1.0f);
        
        // 로그 출력 (약 10프레임마다)
        if (FMath::Fmod(FadeTime, 0.16f) < InDeltaTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("Tick 페이드아웃: %f초 / %f초, Alpha=%f"), 
                FadeTime, FadeOutDuration, Alpha);
        }
        
        // 보더 투명도 설정
        FadeBorder->SetRenderOpacity(Alpha);
        
        // 완료되면 페이드 종료
        if (FadeTime >= FadeOutDuration)
        {
            FadeBorder->SetRenderOpacity(0.0f);
            bIsFading = false;
            UE_LOG(LogTemp, Warning, TEXT("Tick 페이드아웃 완료"));
            
            // 로고와 메뉴 페이드인 시작
            StartLogoAndMenuFadeIn();
        }
    }
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