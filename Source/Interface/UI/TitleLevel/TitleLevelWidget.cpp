#include "TitleLevelWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h" // UTextBlock 추가
#include "Blueprint/WidgetTree.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/Border.h"

UTitleLevelWidget::UTitleLevelWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
    FadeOutDuration = 1.5f;  // 페이드 아웃 지속 시간 (초)
    
    // Tick 활성화 (중요!)
    bHasScriptImplementedTick = true;
}

// 위젯이 실제로 생성될 때 호출됨
void UTitleLevelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 검은색 페이드 스크린을 독립적으로 생성
    UWorld* World = GetWorld();
    if (World && World->GetGameViewport())
    {
        // 1. 독립 보더 위젯 생성
        UBorder* FadeScreen = NewObject<UBorder>(GetTransientPackage(), TEXT("FadeScreen"));
        FadeScreen->SetBrushColor(FLinearColor(0, 0, 0, 1));
        FadeScreen->SetRenderOpacity(1.0f); // 완전 불투명 시작
        
        // 2. SBorder 래핑하여 뷰포트에 직접 추가
        TSharedRef<SBorder> SlateWidget = StaticCastSharedRef<SBorder>(FadeScreen->TakeWidget());
        UGameViewportClient* ViewportClient = World->GetGameViewport(); // FViewportClient → UGameViewportClient로 수정
        ViewportClient->AddViewportWidgetContent(
            SNew(SOverlay)
            +SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SlateWidget
            ],
            999999 // 매우 높은 ZOrder로 추가
        );
        
        // 3. 참조 저장
        FadeBorder = FadeScreen;
        
        UE_LOG(LogTemp, Warning, TEXT("독립 페이드 스크린 생성 완료 - Opacity=%f, ZOrder=999999"), 
               FadeBorder->GetRenderOpacity());
    }
    
    // 4. 약간의 지연 후 초기화 진행
    FTimerHandle InitHandle;
    GetWorld()->GetTimerManager().SetTimer(InitHandle, this, &UTitleLevelWidget::InitializeTitleWidget, 0.1f, false);
}

void UTitleLevelWidget::InitializeTitleWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("TitleLevelWidget::InitializeTitleWidget 진입"));
    
    // 흰색 테스트 텍스트 추가 (페이드 스크린이 실제로 보이는지 확인)
    UTextBlock* TestText = NewObject<UTextBlock>(this);
    TestText->SetText(FText::FromString(TEXT("페이드 테스트 중...")));
    TestText->SetColorAndOpacity(FLinearColor::White);
    
    // 1. 기존 Renderer 활용해서 이미지 생성
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
    
    // 페이드 아웃 시작
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