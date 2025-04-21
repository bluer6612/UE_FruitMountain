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
    // 1. 이미 유효한 인스턴스가 있으면 바로 반환 (빠른 경로)
    if (Instance && IsValid(Instance))
    {
        // 뷰포트에 없는 경우에만 추가
        if (!Instance->IsInViewport())
        {
            Instance->AddToViewport(10001);
        }
        return Instance;
    }
    
    // 2. 인스턴스 재설정 (필요한 경우)
    if (Instance)
    {
        Instance = nullptr;
    }
    
    // 3. 필요한 리소스 체크 (World, Controller, WidgetClass)
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateScoreWidget: World가 유효하지 않음!"));
        return nullptr;
    }
    
    APlayerController* Controller = World->GetFirstPlayerController();
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateScoreWidget: PlayerController가 유효하지 않음!"));
        return nullptr;
    }
    
    // 4. 클래스 로드 (없는 경우에만)
    if (!ScoreWidgetClass)
    {
        static const TCHAR* BlueprintPath = TEXT("/Game/UI/PlayLevel/BP_UI_Play_GetScore.BP_UI_Play_GetScore_C");
        ScoreWidgetClass = LoadClass<UUserWidget>(nullptr, BlueprintPath);
        
        if (!ScoreWidgetClass)
        {
            UE_LOG(LogTemp, Error, TEXT("블루프린트를 찾을 수 없습니다: %s"), BlueprintPath);
            return nullptr;
        }
    }
    
    // 5. 인스턴스 생성 및 초기화를 한 번에 처리
    Instance = CreateWidget<UScoreDisplayWidget>(Controller, ScoreWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10001);
        Instance->SetVisibility(ESlateVisibility::HitTestInvisible);
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
    
    // 더 진한 노란색으로 변경 (연한 노란색에서 더 선명한 노란색으로)
    FLinearColor BrighterYellow = FLinearColor(1.0f, 0.9f, 0.2f, 1.0f);
    
    // UI_Play_Score의 오른쪽에 배치하기 위한 위치 설정 수정
    UUIHelper::SetupTextBlockStyle(ScoreTextBlock, BrighterYellow, 60, true);
    UUIHelper::SetScoreDisplayPosition(ScoreTextBlock, 750.0f, 100.0f, 200.0f, 80.0f, false); 
    
    UUIHelper::SetupTextBlockStyle(ComboMultiplierTextBlock, BrighterYellow, 42, true);
    UUIHelper::SetScoreDisplayPosition(ComboMultiplierTextBlock, 800.0f, 200.0f, 180.0f, 70.0f, false);
    
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
    
    // 초기 위치 저장
    UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot);
    UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot);
    
    FVector2D ScoreInitialPos = ScoreSlot ? ScoreSlot->GetPosition() : FVector2D(750.0f, 100.0f);
    FVector2D ComboInitialPos = ComboSlot ? ComboSlot->GetPosition() : FVector2D(800.0f, 160.0f);
    
    // 애니메이션 설정
    float FadeDuration = 1.0f;
    float FadeInterval = 0.05f;
    int32 FadeSteps = FMath::RoundToInt(FadeDuration / FadeInterval);
    float FadeStep = 1.0f / FadeSteps;
    
    // 총 이동 거리 (왼쪽으로 100픽셀 이동)
    float TotalMoveDistance = -100.0f;
    float MoveStep = TotalMoveDistance / FadeSteps;
    
    FTimerDelegate FadeDelegate;
    FadeDelegate.BindLambda([this, FadeStep, FadeSteps, ScoreSlot, ComboSlot, ScoreInitialPos, ComboInitialPos, MoveStep]() {
        static int32 CurrentStep = 0;
        CurrentStep++;
        
        // 투명도 계산
        float Alpha = 1.0f - (CurrentStep * FadeStep);
        
        // 왼쪽으로 이동하는 거리 계산
        float CurrentMoveX = CurrentStep * MoveStep;
        
        // 스코어 텍스트 업데이트
        if (ScoreTextBlock && ScoreSlot)
        {
            // 투명도 조절
            FLinearColor TextColor = ScoreTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            TextColor.A = Alpha;
            ScoreTextBlock->SetColorAndOpacity(TextColor);
            
            // 위치 이동
            FVector2D NewPos = ScoreInitialPos;
            NewPos.X += CurrentMoveX; // X 좌표를 왼쪽으로 이동
            ScoreSlot->SetPosition(NewPos);
        }
        
        // 콤보 텍스트 업데이트
        if (ComboMultiplierTextBlock && ComboSlot)
        {
            // 투명도 조절
            FLinearColor ComboColor = ComboMultiplierTextBlock->GetColorAndOpacity().GetSpecifiedColor();
            ComboColor.A = Alpha;
            ComboMultiplierTextBlock->SetColorAndOpacity(ComboColor);
            
            // 위치 이동
            FVector2D NewPos = ComboInitialPos;
            NewPos.X += CurrentMoveX; // X 좌표를 왼쪽으로 이동
            ComboSlot->SetPosition(NewPos);
        }
        
        // 애니메이션 종료
        if (CurrentStep >= FadeSteps)
        {
            CurrentStep = 0;
            GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
            
            // 텍스트 숨기기
            ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
            ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
            
            // 원래 위치와 불투명도로 복원 (다음 표시를 위해)
            if (ScoreSlot) ScoreSlot->SetPosition(ScoreInitialPos);
            if (ComboSlot) ComboSlot->SetPosition(ComboInitialPos);
            
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