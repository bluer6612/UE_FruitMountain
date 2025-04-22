#include "ScoreDisplayWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "UIHelper.h"
#include "ScoreWidgetAnimator.h"

// 정적 인스턴스 초기화
UScoreDisplayWidget* UScoreDisplayWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UScoreDisplayWidget::ScoreWidgetClass = nullptr;

// 정적 상수 초기화
const FVector2D UScoreDisplayWidget::SCORE_TEXT_POS = FVector2D(650.0f, 75.0f);
const FVector2D UScoreDisplayWidget::COMBO_TEXT_POS = FVector2D(690.0f, 135.0f);
const FLinearColor UScoreDisplayWidget::BRIGHT_YELLOW_COLOR = FLinearColor(1.0f, 0.9f, 0.6f, 1.0f);

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
    APlayerController* PC = UUIHelper::GetValidPlayerController(WorldContextObject);
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

void UScoreDisplayWidget::InitializeTextBlocks()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("ScoreDisplayWidget: 텍스트 블록 바인딩 실패!"));
        return;
    }
    
    // 절대 좌표로 직접 지정
    SetupTextBlock(ScoreTextBlock, BRIGHT_YELLOW_COLOR, 48, SCORE_TEXT_POS); 
    SetupTextBlock(ComboMultiplierTextBlock, BRIGHT_YELLOW_COLOR, 42, COMBO_TEXT_POS);

    // 초기에 텍스트 블록 숨기기
    ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
    ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    
    UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 텍스트 블록 위치 절대좌표로 설정됨"));
}

// SetupTextBlock 함수도 UIHelper 함수 사용으로 간소화
void UScoreDisplayWidget::SetupTextBlock(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, FVector2D Pos)
{
    // 스타일 설정
    UUIHelper::SetupTextBlockStyle(
        TextBlock, 
        Color, 
        FontSize, 
        true,                              // 그림자 사용
        FLinearColor(0.0f, 0.0f, 0.0f, 0.5f),  // 그림자 색상
        FVector2D(1.5f, 1.5f),            // 그림자 오프셋
        true,                              // 볼드체
        false,                             // 자동 줄바꿈 안함
        ESlateVisibility::Hidden           // 초기에 숨김
    );
    
    // 위치 설정
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
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("텍스트 블록이 유효하지 않음!"));
        return;
    }
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World를 가져올 수 없음!"));
        return;
    }
    
    // 총 점수는 계속 누적
    TotalScoreGain += Score;
    
    // 콤보가 끊어진 경우 점수 초기화, 아니면 누적
    if (ComboCount == 1 && CurrentComboMultiplier > 1.0f)
    {
        // 콤보가 끊어진 경우 (이전에 콤보 중이었다가 1로 떨어진 경우)
        CurrentScoreGain = Score;
    }
    else
    {
        // 콤보가 유지되거나 증가하는 경우 - 점수 누적
        CurrentScoreGain += Score;
    }
    
    // 이전 애니메이션 취소 및 텍스트 블록 속성 초기화
    if (WidgetAnimator)
    {
        WidgetAnimator->CancelAnimation();
    }
    
    // 색상 및 투명도 명시적으로 초기화
    if (ScoreTextBlock)
    {
        ScoreTextBlock->SetColorAndOpacity(BRIGHT_YELLOW_COLOR);
    }
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetColorAndOpacity(BRIGHT_YELLOW_COLOR);
    }
    
    // 누적 점수 표시
    FString ScoreText = FString::Printf(TEXT("+%d"), CurrentScoreGain);
    ScoreTextBlock->SetText(FText::FromString(ScoreText));
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    // 콤보 텍스트 업데이트
    CurrentComboMultiplier = ComboMultiplier;
    if (ComboCount >= 2)
    {
        FString ComboText = FString::Printf(TEXT("X%.1f"), CurrentComboMultiplier);
        ComboMultiplierTextBlock->SetText(FText::FromString(ComboText));
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
        ComboMultiplierTextBlock->SetColorAndOpacity(BRIGHT_YELLOW_COLOR); // 불투명도 초기화
    }
    else
    {
        ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 애니메이션 시작 전에 텍스트 블록 위치 재확인
    UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot);
    if (ScoreSlot)
    {
        // 올바른 위치에 있는지 확인하고, 필요하면 강제로 설정
        if (ScoreSlot->GetPosition() != SCORE_TEXT_POS)
        {
            ScoreSlot->SetPosition(SCORE_TEXT_POS);
        }
    }
    
    UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot);
    if (ComboSlot)
    {
        if (ComboSlot->GetPosition() != COMBO_TEXT_POS)
        {
            ComboSlot->SetPosition(COMBO_TEXT_POS);
        }
    }
    
    // 애니메이션 시작
    if (WidgetAnimator)
    {
        WidgetAnimator->StartFadeOutAnimation(this);
    }
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

// 정적 함수 구현
bool UScoreDisplayWidget::IsInstanceValid()
{
    return UUIHelper::IsWidgetInstanceValid(Instance);
}