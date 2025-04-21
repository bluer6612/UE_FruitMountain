#include "ScoreDisplayWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "UIHelper.h"

// 정적 인스턴스 초기화
UScoreDisplayWidget* UScoreDisplayWidget::Instance = nullptr;

UScoreDisplayWidget* UScoreDisplayWidget::CreateScoreWidget(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;
    
    // 인스턴스 유효성 검사
    if (Instance && IsValid(Instance) && Instance->GetIsVisible())
    {
        if (!Instance->IsInViewport())
        {
            Instance->AddToViewport(10001); // TextureDisplayWidget보다 높은 Z-order
        }
        return Instance;
    }
    else if (Instance)
    {
        // 기존 인스턴스가 무효하면 null로 설정
        Instance = nullptr;
    }

    // 새 인스턴스 생성
    APlayerController* Controller = World->GetFirstPlayerController();
    if (!Controller) return nullptr;
    
    Instance = CreateWidget<UScoreDisplayWidget>(Controller, UScoreDisplayWidget::StaticClass());
    if (Instance)
    {
        Instance->AddToViewport(10001);
        UE_LOG(LogTemp, Display, TEXT("ScoreDisplayWidget: 새 인스턴스 생성 성공"));
    }
    
    return Instance;
}

void UScoreDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 업데이트
    Instance = this;
    
    // 루트 캔버스 가져오기
    RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
    if (!RootCanvas && WidgetTree)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
        if (RootCanvas)
        {
            WidgetTree->RootWidget = RootCanvas;
        }
    }
    
    // 점수 텍스트 블록 생성
    if (WidgetTree && RootCanvas)
    {
        // 점수 텍스트 블록 생성
        ScoreTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (ScoreTextBlock)
        {
            UCanvasPanelSlot* ScoreTextSlot = RootCanvas->AddChildToCanvas(ScoreTextBlock);
            
            // 텍스트 스타일 설정
            ScoreTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 0.8f, 0.2f, 1.0f)); // 노란색
            ScoreTextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 42));
            ScoreTextBlock->SetShadowOffset(FVector2D(2.0f, 2.0f));
            ScoreTextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
            ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden); // 초기에는 숨김
            
            // 위치 설정 - 화면 중앙 상단에 배치
            if (ScoreTextSlot)
            {
                ScoreTextSlot->SetPosition(FVector2D(200.0f, 100.0f));
                ScoreTextSlot->SetSize(FVector2D(200.0f, 50.0f));
                ScoreTextSlot->SetAlignment(FVector2D(0.5f, 0.0f)); // 가운데 정렬
            }
        }
        
        // 콤보 배율 텍스트 블록 생성
        ComboMultiplierTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        if (ComboMultiplierTextBlock)
        {
            UCanvasPanelSlot* ComboTextSlot = RootCanvas->AddChildToCanvas(ComboMultiplierTextBlock);
            
            // 텍스트 스타일 설정
            ComboMultiplierTextBlock->SetColorAndOpacity(FLinearColor(0.2f, 1.0f, 0.5f, 1.0f)); // 초록색
            ComboMultiplierTextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 36));
            ComboMultiplierTextBlock->SetShadowOffset(FVector2D(2.0f, 2.0f));
            ComboMultiplierTextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
            ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden); // 초기에는 숨김
            
            // 위치 설정 - 점수 텍스트 아래에 배치
            if (ComboTextSlot)
            {
                ComboTextSlot->SetPosition(FVector2D(200.0f, 150.0f));
                ComboTextSlot->SetSize(FVector2D(200.0f, 50.0f));
                ComboTextSlot->SetAlignment(FVector2D(0.5f, 0.0f)); // 가운데 정렬
            }
        }
    }
}

void UScoreDisplayWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

void UScoreDisplayWidget::DisplayScoreGain(int32 Score, int32 ComboCount, float ComboMultiplier)
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock || !GetWorld())
        return;
    
    // 점수 값 설정
    PendingScoreGain += Score;
    FString ScoreText = FString::Printf(TEXT("+%d 점"), PendingScoreGain);
    ScoreTextBlock->SetText(FText::FromString(ScoreText));
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    // 기존 타이머 취소
    if (GetWorld()->GetTimerManager().IsTimerActive(ScoreAnimTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
    }
    
    // 연쇄(콤보) 배율 표시
    CurrentComboMultiplier = ComboMultiplier;
    if (ComboCount >= 2) // 2개 이상이면 콤보로 간주
    {
        FString ComboText = FString::Printf(TEXT("x%.1f"), CurrentComboMultiplier);
        ComboMultiplierTextBlock->SetText(FText::FromString(ComboText));
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 텍스트 활성화 표시
    bScoreTextActive = true;
    
    // 점수 텍스트 애니메이션 - 2초 후 페이드 아웃
    GetWorld()->GetTimerManager().SetTimer(
        ScoreAnimTimerHandle, 
        this, 
        &UScoreDisplayWidget::FadeOutScoreText, 
        2.0f, 
        false
    );
}

void UScoreDisplayWidget::FadeOutScoreText()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock || !GetWorld())
        return;
    
    // 텍스트를 점차 페이드 아웃하기 위한 구조
    float FadeDuration = 1.0f; // 1초 동안 페이드 아웃
    float FadeInterval = 0.05f; // 0.05초마다 업데이트
    int32 FadeSteps = FMath::RoundToInt(FadeDuration / FadeInterval);
    float FadeStep = 1.0f / FadeSteps;
    
    // 페이드 아웃 함수를 반복적으로 호출
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, FadeStep, FadeSteps]() {
        static int32 CurrentStep = 0;
        CurrentStep++;
        
        // 점차 투명하게 만들기
        float Alpha = 1.0f - (CurrentStep * FadeStep);
        if (ScoreTextBlock)
        {
            FLinearColor TextColor = ScoreTextBlock->ColorAndOpacity.GetSpecifiedColor();
            TextColor.A = Alpha;
            ScoreTextBlock->SetColorAndOpacity(TextColor);
        }
        
        if (ComboMultiplierTextBlock)
        {
            FLinearColor ComboColor = ComboMultiplierTextBlock->ColorAndOpacity.GetSpecifiedColor();
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
        }
        
        // 페이드 아웃 완료 후 초기화
        if (CurrentStep >= FadeSteps)
        {
            CurrentStep = 0;
            GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
            
            // 텍스트 숨기기 및 초기화
            ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
            ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
            
            // 불투명도 복원 (다음 사용을 위해)
            FLinearColor TextColor = ScoreTextBlock->ColorAndOpacity.GetSpecifiedColor();
            TextColor.A = 1.0f;
            ScoreTextBlock->SetColorAndOpacity(TextColor);
            
            FLinearColor ComboColor = ComboMultiplierTextBlock->ColorAndOpacity.GetSpecifiedColor();
            ComboColor.A = 1.0f;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
            
            // 값 초기화
            PendingScoreGain = 0;
            bScoreTextActive = false;
        }
    });
    
    GetWorld()->GetTimerManager().SetTimer(
        ScoreAnimTimerHandle, 
        FadeDelegate, 
        FadeInterval, 
        true
    );
}