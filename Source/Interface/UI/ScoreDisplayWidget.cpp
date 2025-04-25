#include "ScoreDisplayWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "UIWidgetUtility.h"
#include "ScoreWidgetAnimator.h"

// 정적 인스턴스 초기화
UScoreDisplayWidget* UScoreDisplayWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UScoreDisplayWidget::ScoreWidgetClass = nullptr;

// 정적 상수 초기화
const FVector2D UScoreDisplayWidget::SCORE_TEXT_POS = FVector2D(650.0f, 75.0f);
const FVector2D UScoreDisplayWidget::COMBO_TEXT_POS = FVector2D(690.0f, 135.0f);
const FLinearColor UScoreDisplayWidget::SCORE_YELLOW_COLOR = FLinearColor(232.0f/255.0f, 229.0f/255.0f, 176.0f/255.0f, 1.0f);

UScoreDisplayWidget::UScoreDisplayWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    TotalScoreGain = 0;
    CurrentScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
    bScoreTextActive = false;
    WidgetAnimator = nullptr;
}

void UScoreDisplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 업데이트
    Instance = this;
    
    // 텍스트 블록 초기화
    InitializeTextBlocks();
    
    // 애니메이터 생성
    WidgetAnimator = NewObject<UScoreWidgetAnimator>(this, UScoreWidgetAnimator::StaticClass());
    WidgetAnimator->SetTextBlocks(ScoreTextBlock, ComboMultiplierTextBlock);
}

void UScoreDisplayWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 애니메이터 정리
    if (WidgetAnimator)
    {
        // 모든 대리자 초기화
        WidgetAnimator->CancelAnimation();
        WidgetAnimator = nullptr;
    }
    
    // 인스턴스 참조 해제
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

UScoreDisplayWidget* UScoreDisplayWidget::CreateScoreWidget(UObject* WorldContextObject)
{
    static const FString BlueprintPath = TEXT("/Game/UI/PlayLevel/BP_UI_Play_GetScore.BP_UI_Play_GetScore_C");
    
    // 이미 인스턴스가 있는지 확인
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 플레이어 컨트롤러 확인
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (!PC)
    {
        return nullptr;
    }
    
    // 위젯 클래스 로드
    if (!ScoreWidgetClass)
    {
        ScoreWidgetClass = LoadClass<UUserWidget>(nullptr, *BlueprintPath);
        if (!ScoreWidgetClass)
        {
            return nullptr;
        }
    }
    
    // 인스턴스 생성
    Instance = CreateWidget<UScoreDisplayWidget>(PC, ScoreWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10001);
        Instance->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    return Instance;
}

void UScoreDisplayWidget::BeginDestroy()
{
    // 정적 인스턴스 참조 해제 (중요)
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    Super::BeginDestroy();
}

void UScoreDisplayWidget::InitializeTextBlocks()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("ScoreDisplayWidget: 텍스트 블록 바인딩 실패!"));
        return;
    }
    
    // 절대 좌표로 직접 지정
    SetupTextBlock(ScoreTextBlock, SCORE_YELLOW_COLOR, 48, SCORE_TEXT_POS); 
    SetupTextBlock(ComboMultiplierTextBlock, SCORE_YELLOW_COLOR, 42, COMBO_TEXT_POS);

    // 초기에 텍스트 블록 숨기기
    ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
    ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    
    UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 텍스트 블록 위치 절대좌표로 설정됨"));
}

void UScoreDisplayWidget::SetupTextBlock(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, FVector2D Pos)
{
    // 스타일 설정
    UUIWidgetUtility::SetupTextBlockStyle(
        TextBlock, 
        Color, 
        FontSize,
        true,                              // 볼드체
        false,                             // 자동 줄바꿈 안함
        ESlateVisibility::Hidden           // 초기에 숨김
    );
    
    // 위치 설정 (기존 코드 유지)
    UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(TextBlock->Slot);
    if (TextSlot)
    {
        TextSlot->SetAnchors(FAnchors());
        TextSlot->SetAlignment(FVector2D::ZeroVector);
        TextSlot->SetPosition(Pos);
        TextSlot->SetSize(FVector2D(TextBlock == ScoreTextBlock ? 200.0f : 180.0f, 
                           TextBlock == ScoreTextBlock ? 80.0f : 70.0f));
    }
}

void UScoreDisplayWidget::DisplayScoreGain(int32 Score, int32 ComboCount, float ComboMultiplier)
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock || !WidgetAnimator)
    {
        UE_LOG(LogTemp, Error, TEXT("DisplayScoreGain: 위젯 구성요소가 null입니다"));
        return;
    }
    
    // 기존 애니메이션 취소
    if (WidgetAnimator)
    {
        WidgetAnimator->CancelAnimation();
    }
    
    // 현재 콤보 멀티플라이어 저장
    CurrentComboMultiplier = ComboMultiplier;
    
    // AnimateScoreText와 AnimateComboText 함수에 위임
    WidgetAnimator->AnimateScoreText(Score);
    WidgetAnimator->AnimateComboText(ComboCount, ComboMultiplier);
    
    // 애니메이션 시작
    if (WidgetAnimator && ScoreTextBlock->GetWorld())
    {
        WidgetAnimator->StartFadeOutAnimation(ScoreTextBlock->GetWorld(), 1.5f);
    }
    
    // 디버그 로그
    UE_LOG(LogTemp, Display, TEXT("점수 표시: %d (콤보: %d, 배율: %.1f)"), 
           Score, ComboCount, ComboMultiplier);
    
    bScoreTextActive = true;
}

void UScoreDisplayWidget::ResetComboDisplay()
{
    // 현재 콤보 관련 변수 초기화
    CurrentScoreGain = 0;
    CurrentComboMultiplier = 1.0f;
    
    // 콤보 텍스트 숨기기
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
}