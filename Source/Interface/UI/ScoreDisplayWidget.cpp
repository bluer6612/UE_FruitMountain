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
const FLinearColor UScoreDisplayWidget::SCORE_YELLOW_COLOR = FLinearColor(232.0f/255.0f, 235.0f/255.0f, 141.0f/255.0f, 1.0f);

UScoreDisplayWidget::UScoreDisplayWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

void UScoreDisplayWidget::BeginDestroy()
{
    // 정적 인스턴스 참조 해제 (중요)
    if (Instance == this)
    {
        Instance = nullptr;
    }
    
    Super::BeginDestroy();
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

void UScoreDisplayWidget::InitializeTextBlocks()
{
    // 텍스트 블록 생성 및 설정
    if (!ScoreTextBlock || !ComboMultiplierTextBlock)
    {
        UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
        if (RootCanvas)
        {
            // 점수 텍스트 블록 생성
            ScoreTextBlock = NewObject<UTextBlock>(this, TEXT("ScoreTextBlock"));
            if (ScoreTextBlock)
            {
                RootCanvas->AddChild(ScoreTextBlock);
                
                // 스타일 설정
                UUIWidgetUtility::SetupTextBlockStyle(
                    ScoreTextBlock, 
                    SCORE_YELLOW_COLOR, 
                    46.0f,      // 폰트 크기 
                    true,       // 볼드체
                    false,      // 자동 줄바꿈 안함
                    ESlateVisibility::Hidden // 초기에는 숨김
                );
                
                // 위치 설정
                UCanvasPanelSlot* ScoreSlot = Cast<UCanvasPanelSlot>(ScoreTextBlock->Slot);
                if (ScoreSlot)
                {
                    ScoreSlot->SetAnchors(FAnchors());
                    ScoreSlot->SetAlignment(FVector2D::ZeroVector);
                    ScoreSlot->SetPosition(SCORE_TEXT_POS);
                    ScoreSlot->SetSize(FVector2D(200.0f, 80.0f));
                }
            }
            
            // 콤보 텍스트 블록 생성
            ComboMultiplierTextBlock = NewObject<UTextBlock>(this, TEXT("ComboTextBlock"));
            if (ComboMultiplierTextBlock)
            {
                RootCanvas->AddChild(ComboMultiplierTextBlock);
                
                // 스타일 설정
                UUIWidgetUtility::SetupTextBlockStyle(
                    ComboMultiplierTextBlock, 
                    SCORE_YELLOW_COLOR,
                    36.0f,      // 콤보 텍스트는 좀 더 작은 크기
                    true,       // 볼드체
                    false,      // 자동 줄바꿈 안함
                    ESlateVisibility::Hidden // 초기에는 숨김
                );
                
                // 위치 설정
                UCanvasPanelSlot* ComboSlot = Cast<UCanvasPanelSlot>(ComboMultiplierTextBlock->Slot);
                if (ComboSlot)
                {
                    ComboSlot->SetAnchors(FAnchors());
                    ComboSlot->SetAlignment(FVector2D::ZeroVector);
                    ComboSlot->SetPosition(COMBO_TEXT_POS);
                    ComboSlot->SetSize(FVector2D(180.0f, 70.0f));
                }
            }
        }
    }
    
    // 애니메이터 생성
    if (!WidgetAnimator)
    {
        WidgetAnimator = NewObject<UScoreWidgetAnimator>(this);
        WidgetAnimator->Initialize(ScoreTextBlock, ComboMultiplierTextBlock);
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

bool UScoreDisplayWidget::IsInstanceValid()
{
    return Instance && IsValid(Instance) && Instance->IsInViewport();
}

void UScoreDisplayWidget::ClearInstance()
{
    if (Instance)
    {
        if (Instance->IsInViewport())
        {
            Instance->RemoveFromParent();
        }
        Instance = nullptr;
    }
    
    UE_LOG(LogTemp, Display, TEXT("ScoreDisplayWidget: 인스턴스 제거됨"));
}