#include "ScoreDisplayWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "UIHelper.h"

// 정적 인스턴스 초기화
UScoreDisplayWidget* UScoreDisplayWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UScoreDisplayWidget::ScoreWidgetClass = nullptr;

// 기존 생성자 코드 그대로 유지
UScoreDisplayWidget::UScoreDisplayWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 정적 인스턴스를 생성자에서 설정하지 않음 (NativeConstruct에서 설정)
}

UScoreDisplayWidget* UScoreDisplayWidget::CreateScoreWidget(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateScoreWidget: World가 유효하지 않음!"));
        return nullptr;
    }
    
    // 블루프린트 클래스 로드 시 자세한 디버그
    if (!ScoreWidgetClass)
    {
        FString BlueprintPath = TEXT("/Game/UI/PlayLevel/BP_UI_Play_GetScore.BP_UI_Play_GetScore_C");
        UE_LOG(LogTemp, Warning, TEXT("블루프린트 로드 시도: %s"), *BlueprintPath);
        
        ScoreWidgetClass = LoadClass<UUserWidget>(nullptr, *BlueprintPath);
        
        if (!ScoreWidgetClass)
        {
            UE_LOG(LogTemp, Error, TEXT("블루프린트를 찾을 수 없습니다! 경로를 확인하세요: %s"), *BlueprintPath);
            return nullptr;
        }
        
        UE_LOG(LogTemp, Display, TEXT("블루프린트 로드 성공: %s"), *ScoreWidgetClass->GetName());
    }
    
    if (Instance && IsValid(Instance))
    {
        if (!Instance->IsInViewport())
        {
            Instance->AddToViewport(10001);
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
    
    if (!Controller)
    {
        return nullptr;
    }
    
    Instance = CreateWidget<UScoreDisplayWidget>(Controller, ScoreWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10001);
        Instance->SetVisibility(ESlateVisibility::HitTestInvisible); // 강제로 보이게 설정
        
        // 텍스트 블록도 강제로 표시
        if (Instance->ScoreTextBlock)
        {
            Instance->ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
            Instance->ScoreTextBlock->SetText(FText::FromString(TEXT("+TEST")));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 새 인스턴스 생성 및 강제 표시 설정"));
    }
    
    return Instance;
}

void UScoreDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 업데이트
    Instance = this;
    
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("ScoreDisplayWidget: 텍스트 블록 바인딩 실패!"));
        return;
    }
    
    FLinearColor LightYellow = FLinearColor(1.0f, 1.0f, 0.8f, 1.0f);
    
    // UI_Play_Score의 오른쪽에 배치하기 위한 위치 설정 수정
    // UI_Play_Score는 TopLeft에 (40, 30) 위치, 크기는 504x253
    UUIHelper::SetupTextBlockStyle(ScoreTextBlock, LightYellow, 60, true);
    UUIHelper::SetScoreDisplayPosition(ScoreTextBlock, 750.0f, 100.0f, 200.0f, 80.0f, false); // false = 오른쪽 정렬 아님
    
    UUIHelper::SetupTextBlockStyle(ComboMultiplierTextBlock, LightYellow, 42, true);
    UUIHelper::SetScoreDisplayPosition(ComboMultiplierTextBlock, 800.0f, 160.0f, 180.0f, 70.0f, false); // false = 오른쪽 정렬 아님
    
    UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: UI_Play_Score 오른쪽에 위치 설정 완료"));
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
    UE_LOG(LogTemp, Warning, TEXT("DisplayScoreGain 호출됨: 점수=%d, 콤보=%d, 배율=%.1f"), 
           Score, ComboCount, ComboMultiplier);
    
    if (!IsValid(ScoreTextBlock) || !IsValid(ComboMultiplierTextBlock))
    {
        UE_LOG(LogTemp, Error, TEXT("텍스트 블록이 유효하지 않음!"));
        return;
    }
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World를 가져올 수 없음!"));
        return;
    }
    
    PendingScoreGain += Score;
    FString ScoreText = FString::Printf(TEXT("+%d"), PendingScoreGain);
    ScoreTextBlock->SetText(FText::FromString(ScoreText));
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    if (GetWorld()->GetTimerManager().IsTimerActive(ScoreAnimTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
    }
    
    CurrentComboMultiplier = ComboMultiplier;
    if (ComboCount >= 2)
    {
        FString ComboText = FString::Printf(TEXT("x%.1f"), CurrentComboMultiplier);
        ComboMultiplierTextBlock->SetText(FText::FromString(ComboText));
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    bScoreTextActive = true;
    
    GetWorld()->GetTimerManager().SetTimer(
        ScoreAnimTimerHandle, 
        this, 
        &UScoreDisplayWidget::FadeOutScoreText, 
        2.0f, 
        false
    );
    
    UE_LOG(LogTemp, Warning, TEXT("점수 표시 설정 완료: '%s'"), *ScoreText);
}

void UScoreDisplayWidget::FadeOutScoreText()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock || !GetWorld())
    {
        return;
    }
    
    float FadeDuration = 1.0f;
    float FadeInterval = 0.05f;
    int32 FadeSteps = FMath::RoundToInt(FadeDuration / FadeInterval);
    float FadeStep = 1.0f / FadeSteps;
    
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, FadeStep, FadeSteps]() {
        static int32 CurrentStep = 0;
        CurrentStep++;
        
        float Alpha = 1.0f - (CurrentStep * FadeStep);
        if (ScoreTextBlock)
        {
            FLinearColor TextColor = ScoreTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            TextColor.A = Alpha;
            ScoreTextBlock->SetColorAndOpacity(TextColor);
        }
        
        if (ComboMultiplierTextBlock)
        {
            FLinearColor ComboColor = ComboMultiplierTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
        }
        
        if (CurrentStep >= FadeSteps)
        {
            CurrentStep = 0;
            GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
            
            ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
            ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
            
            FLinearColor TextColor = ScoreTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            TextColor.A = 1.0f;
            ScoreTextBlock->SetColorAndOpacity(TextColor);
            
            FLinearColor ComboColor = ComboMultiplierTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            ComboColor.A = 1.0f;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
            
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

void UScoreDisplayWidget::ShowTestScore(UObject* WorldContextObject, int32 Score)
{
    UScoreDisplayWidget* Widget = CreateScoreWidget(WorldContextObject);
    if (Widget)
    {
        Widget->DisplayScoreGain(Score, 2, 1.1f);
        UE_LOG(LogTemp, Warning, TEXT("테스트 점수 %d 표시됨"), Score);
    }
}