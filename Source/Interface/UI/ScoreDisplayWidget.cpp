#include "ScoreDisplayWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "UIHelper.h"

// 정적 인스턴스 초기화
UScoreDisplayWidget* UScoreDisplayWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UScoreDisplayWidget::ScoreWidgetClass = nullptr;

UScoreDisplayWidget* UScoreDisplayWidget::CreateScoreWidget(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;
    
    // 블루프린트 클래스 로드 부분
    if (!ScoreWidgetClass)
    {
        // 블루프린트 위젯 클래스 로드 - 수정된 경로 적용
        ScoreWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/PlayLevel/BP_UI_Play_GetScore.BP_UI_Play_GetScore_C"));
        
        if (!ScoreWidgetClass)
        {
            UE_LOG(LogTemp, Error, TEXT("BP_UI_Play_GetScore 블루프린트를 찾을 수 없습니다!"));
            return nullptr;
        }
    }
    
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
    
    // ScoreTextBlock과 ComboMultiplierTextBlock은 이미 블루프린트에서 생성되어 있음
    // 블루프린트에서 바인딩된 텍스트 블록이 존재하는지 확인
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("ScoreDisplayWidget: 텍스트 블록 바인딩 실패! 위젯 블루프린트에서 변수 이름이 정확한지 확인하세요."));
        return;
    }
    
    // 2-2. 텍스트 블록 스타일 설정
    // ScoreTextBlock 설정
    ScoreTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.8f, 1.0f)); // 연한 노란색
    ScoreTextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 60)); // 크기 유지
    ScoreTextBlock->SetShadowOffset(FVector2D(2.0f, 2.0f));
    ScoreTextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
    ScoreTextBlock->SetText(FText::FromString(TEXT("")));
    ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);

    // ComboMultiplierTextBlock 설정 - 색상도 연한 노란색으로 통일
    ComboMultiplierTextBlock->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.8f, 1.0f)); // 연한 노란색
    ComboMultiplierTextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 42)); // 약간 크기 증가
    ComboMultiplierTextBlock->SetShadowOffset(FVector2D(2.0f, 2.0f));
    ComboMultiplierTextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
    ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    
    // 2-3. 위치 설정 (CanvasPanelSlot 사용)
    UCanvasPanelSlot* ScoreTextSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot);
    if (ScoreTextSlot)
    {
        // UI_Play_Score의 우측에 배치
        ScoreTextSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f)); // 화면 우측 앵커
        ScoreTextSlot->SetAlignment(FVector2D(1.0f, 0.5f)); // 우측 정렬
        ScoreTextSlot->SetPosition(FVector2D(-20.0f, 120.0f)); // UI_Play_Score 우측에 20px 간격
        ScoreTextSlot->SetSize(FVector2D(200.0f, 80.0f)); // 사이즈 설정
        
        UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 점수 텍스트 위치 설정 - UI_Play_Score 우측"));
    }
    
    UCanvasPanelSlot* ComboTextSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot);
    if (ComboTextSlot)
    {
        // 점수 텍스트 아래에 배치
        ComboTextSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f)); // 화면 우측 앵커
        ComboTextSlot->SetAlignment(FVector2D(1.0f, 0.5f)); // 우측 정렬
        ComboTextSlot->SetPosition(FVector2D(-20.0f, 170.0f)); // 점수 텍스트 아래
        ComboTextSlot->SetSize(FVector2D(180.0f, 70.0f)); // 작은 크기
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 위젯 초기화 완료"));
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
    
    // 점수 값 설정
    PendingScoreGain += Score;
    FString ScoreText = FString::Printf(TEXT("+%d"), PendingScoreGain);
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
    
    // 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("점수 표시 설정 완료: '%s'"), *ScoreText);
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

void UScoreDisplayWidget::ShowTestScore(UObject* WorldContextObject, int32 Score)
{
    UScoreDisplayWidget* Widget = CreateScoreWidget(WorldContextObject);
    if (Widget)
    {
        Widget->DisplayScoreGain(Score, 2, 1.1f);
        UE_LOG(LogTemp, Warning, TEXT("테스트 점수 %d 표시됨"), Score);
    }
}