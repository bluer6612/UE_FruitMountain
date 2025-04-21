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
const FVector2D UScoreDisplayWidget::SCORE_TEXT_POS = FVector2D(675.0f, 90.0f);
const FVector2D UScoreDisplayWidget::COMBO_TEXT_POS = FVector2D(725.0f, 165.0f);

UScoreDisplayWidget::UScoreDisplayWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PendingScoreGain = 0;
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
    
    // 위치 초기화 확인 로그
    if (ScoreTextBlock && Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot))
    {
        FVector2D CurrentPos = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot)->GetPosition();
        UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 최종 위치 확인 (%.1f, %.1f)"), CurrentPos.X, CurrentPos.Y);
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

void UScoreDisplayWidget::InitializeTextBlocks()
{
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UE_LOG(LogTemp, Error, TEXT("ScoreDisplayWidget: 텍스트 블록 바인딩 실패!"));
        return;
    }
    
    // 연한 노란색
    FLinearColor BrighterYellow = FLinearColor(1.0f, 0.9f, 0.2f, 1.0f);
    
    // 절대 좌표로 직접 지정
    SetupTextBlock(ScoreTextBlock, BrighterYellow, 50, SCORE_TEXT_POS); 
    SetupTextBlock(ComboMultiplierTextBlock, BrighterYellow, 45, COMBO_TEXT_POS);

    // 초기에 텍스트 블록 숨기기
    ScoreTextBlock->SetVisibility(ESlateVisibility::Hidden);
    ComboMultiplierTextBlock->SetVisibility(ESlateVisibility::Hidden);
    
    UE_LOG(LogTemp, Warning, TEXT("ScoreDisplayWidget: 텍스트 블록 위치 절대좌표로 설정됨"));
}

void UScoreDisplayWidget::SetupTextBlock(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, FVector2D Pos)
{
    // 스타일 설정
    UUIHelper::SetupTextBlockStyle(TextBlock, Color, FontSize, true);
    
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
    UE_LOG(LogTemp, Warning, TEXT("DisplayScoreGain 호출됨: 점수=%d, 콤보=%d, 배율=%.1f"), 
           Score, ComboCount, ComboMultiplier);
    
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
    
    // 이전 애니메이션 취소 및 텍스트 블록 속성 초기화
    if (WidgetAnimator)
    {
        WidgetAnimator->CancelAnimation();
    }
    
    // 색상 및 투명도 명시적으로 초기화
    FLinearColor BrighterYellow = FLinearColor(1.0f, 0.9f, 0.2f, 1.0f);
    if (ScoreTextBlock)
    {
        ScoreTextBlock->SetColorAndOpacity(BrighterYellow);
    }
    if (ComboMultiplierTextBlock)
    {
        ComboMultiplierTextBlock->SetColorAndOpacity(BrighterYellow);
    }
    
    // 점수 텍스트 업데이트
    PendingScoreGain += Score;
    FString ScoreText = FString::Printf(TEXT("+%d"), PendingScoreGain);
    ScoreTextBlock->SetText(FText::FromString(ScoreText));
    ScoreTextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    // 콤보 텍스트 업데이트
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
    
    // 애니메이션 시작 전에 텍스트 블록 위치 재확인
    UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot);
    if (ScoreSlot)
    {
        // 올바른 위치에 있는지 확인하고, 필요하면 강제로 설정
        if (ScoreSlot->GetPosition() != SCORE_TEXT_POS)
        {
            ScoreSlot->SetPosition(SCORE_TEXT_POS);
            UE_LOG(LogTemp, Warning, TEXT("점수 위치 재조정됨: (%.1f, %.1f)"), SCORE_TEXT_POS.X, SCORE_TEXT_POS.Y);
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
        WidgetAnimator->StartFadeOutAnimation(this, 2.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("점수 표시 설정 완료: '%s'"), *ScoreText);
}

// 정적 함수 구현
bool UScoreDisplayWidget::IsInstanceValid()
{
    return Instance && IsValid(Instance) && Instance->IsInViewport();
}

APlayerController* UScoreDisplayWidget::GetValidPlayerController(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) {
        UE_LOG(LogTemp, Error, TEXT("GetValidPlayerController: World가 유효하지 않음!"));
        return nullptr;
    }
    
    APlayerController* Controller = World->GetFirstPlayerController();
    if (!Controller) {
        UE_LOG(LogTemp, Error, TEXT("GetValidPlayerController: PlayerController가 유효하지 않음!"));
        return nullptr;
    }
    
    return Controller;
}

bool UScoreDisplayWidget::LoadWidgetClassIfNeeded()
{
    if (ScoreWidgetClass) {
        return true;
    }
    
    static const TCHAR* BlueprintPath = TEXT("/Game/UI/PlayLevel/BP_UI_Play_GetScore.BP_UI_Play_GetScore_C");
    ScoreWidgetClass = LoadClass<UUserWidget>(nullptr, BlueprintPath);
    
    if (!ScoreWidgetClass) {
        UE_LOG(LogTemp, Error, TEXT("블루프린트를 찾을 수 없습니다: %s"), BlueprintPath);
        return false;
    }
    
    return true;
}