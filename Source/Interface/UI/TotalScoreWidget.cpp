#include "TotalScoreWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UIHelper.h"
#include "ScoreDisplayWidget.h"
#include "Components/CanvasPanelSlot.h"

// 정적 변수 초기화
UTotalScoreWidget* UTotalScoreWidget::Instance = nullptr;
TSubclassOf<UUserWidget> UTotalScoreWidget::TotalScoreWidgetClass = nullptr;

// 색상 상수 정의
const FLinearColor UTotalScoreWidget::SCORE_BROWN_COLOR = FLinearColor(0.6f, 0.3f, 0.05f, 1.0f); // 갈색
const FLinearColor UTotalScoreWidget::SCORE_SHADOW_COLOR = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f); // 반투명 검은색

UTotalScoreWidget::UTotalScoreWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentTotalScore(0)
    , TargetScore(0)
    , StartScore(0)
    , PendingScore(0)
    , AnimSteps(25)  // 애니메이션 단계 수
    , CurrentStep(0)
    , bAnimating(false)
{
}

void UTotalScoreWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 인스턴스 저장
    Instance = this;
    
    // 텍스트 스타일 설정
    SetupTotalScoreTextStyle();
    
    // UI_Play_Score 위젯 위에 위치하도록 설정
    PositionWidgetAboveScoreDisplay();
    
    // 초기 총점 표시
    if (CurrentTotalScoreTextBlock)
    {
        CurrentTotalScoreTextBlock->SetText(FText::AsNumber(0));
    }
}

void UTotalScoreWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
    }
    
    // 인스턴스 정리
    if (Instance == this)
    {
        Instance = nullptr;
    }
}

// 텍스트 스타일 설정 함수 구현
void UTotalScoreWidget::SetupTotalScoreTextStyle()
{
    if (!CurrentTotalScoreTextBlock)
    {
        return;
    }
    
    // UIHelper 함수를 사용하여 스타일 설정
    UUIHelper::SetupTextBlockStyle(
        CurrentTotalScoreTextBlock,
        SCORE_BROWN_COLOR,          // 갈색 텍스트
        32,                         // 폰트 크기
        true,                       // 그림자 사용
        SCORE_SHADOW_COLOR,         // 그림자 색상
        FVector2D(2.0f, 2.0f),      // 그림자 오프셋
        true,                       // 볼드체
        false,                      // 자동 줄바꿈 안함
        ESlateVisibility::Visible   // 기본 가시성
    );
}

void UTotalScoreWidget::PositionWidgetAboveScoreDisplay()
{
    if (!CurrentTotalScoreTextBlock)
    {
        return;
    }
    
    // UI_Play_Score 위젯 위에 배치 (ScoreDisplayWidget의 위치 참조)
    UCanvasPanelSlot* TotalScoreSlot = Cast<UCanvasPanelSlot>(CurrentTotalScoreTextBlock->Slot);
    if (TotalScoreSlot)
    {
        // UI_Play_Score 위젯(ScoreDisplayWidget)의 위치보다 약간 위에 배치
        FVector2D ScorePosition = UScoreDisplayWidget::SCORE_TEXT_POS;
        
        // 총점 표시는 점수 위치보다 약간 위에 배치
        FVector2D TotalScorePosition = FVector2D(ScorePosition.X - 40.0f, ScorePosition.Y - 50.0f);
        
        // 위치 설정
        TotalScoreSlot->SetPosition(TotalScorePosition);
        TotalScoreSlot->SetSize(FVector2D(200.0f, 40.0f));
        TotalScoreSlot->SetAnchors(FAnchors());
        TotalScoreSlot->SetAlignment(FVector2D::ZeroVector);
    }
}

UTotalScoreWidget* UTotalScoreWidget::CreateTotalScoreWidget(UObject* WorldContextObject)
{
    // 기존 유효 인스턴스 확인
    if (Instance && IsValid(Instance) && Instance->IsInViewport())
    {
        return Instance;
    }
    
    // 플레이어 컨트롤러 확인
    APlayerController* PC = GetValidPlayerController(WorldContextObject);
    if (!PC)
    {
        return nullptr;
    }
    
    // 위젯 클래스 로드
    if (!LoadWidgetClassIfNeeded())
    {
        return nullptr;
    }
    
    // 위젯 생성
    Instance = CreateWidget<UTotalScoreWidget>(PC, TotalScoreWidgetClass);
    if (Instance)
    {
        Instance->AddToViewport(10);
    }
    
    return Instance;
}

bool UTotalScoreWidget::LoadWidgetClassIfNeeded()
{
    if (!TotalScoreWidgetClass)
    {
        TotalScoreWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/PlayLevel/BP_UI_Play_TotalScore.BP_UI_Play_TotalScore_C"));
    }
    return TotalScoreWidgetClass != nullptr;
}

APlayerController* UTotalScoreWidget::GetValidPlayerController(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    return World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
}

void UTotalScoreWidget::UpdateTotalScore(int32 NewScore)
{
    // 점수 업데이트
    CurrentTotalScore = NewScore;
    
    // UI 업데이트
    if (CurrentTotalScoreTextBlock)
    {
        // 점수 표시 형식 변경 (천 단위 구분 기호 추가)
        FNumberFormattingOptions NumberFormat;
        NumberFormat.UseGrouping = true;
        
        CurrentTotalScoreTextBlock->SetText(FText::AsNumber(CurrentTotalScore, &NumberFormat));
    }
}

// 애니메이션과 함께 점수 증가
void UTotalScoreWidget::AnimateScoreIncrease(int32 NewScore)
{
    if (!GetWorld())
    {
        // 월드가 없으면 즉시 업데이트
        UpdateTotalScore(NewScore);
        return;
    }
    
    // 대기중인 점수 업데이트
    PendingScore = NewScore;
    
    // 현재 애니메이션 중이면 타겟 점수만 업데이트
    if (bAnimating)
    {
        TargetScore = PendingScore;
        return;
    }
    
    // 새 애니메이션 시작
    StartScore = CurrentTotalScore;
    TargetScore = PendingScore;
    CurrentStep = 0;
    bAnimating = true;
    
    // 애니메이션 타이머 설정 (0.05초 간격으로 업데이트)
    GetWorld()->GetTimerManager().SetTimer(
        ScoreAnimTimerHandle,
        this,
        &UTotalScoreWidget::UpdateScoreAnimation,
        0.05f,
        true
    );
}

// 애니메이션 타이머 콜백
void UTotalScoreWidget::UpdateScoreAnimation()
{
    if (!bAnimating || !GetWorld())
    {
        return;
    }
    
    CurrentStep++;
    
    // 애니메이션 진행에 따라 현재 점수 계산
    float Progress = FMath::Min(static_cast<float>(CurrentStep) / AnimSteps, 1.0f);
    int32 CurrentAnimScore = StartScore + FMath::RoundToInt((TargetScore - StartScore) * Progress);
    
    // 현재 점수 업데이트
    if (CurrentTotalScoreTextBlock)
    {
        // 점수 표시 형식 변경 (천 단위 구분 기호 추가)
        FNumberFormattingOptions NumberFormat;
        NumberFormat.UseGrouping = true;
        
        CurrentTotalScoreTextBlock->SetText(FText::AsNumber(CurrentAnimScore, &NumberFormat));
    }
    
    // 애니메이션 종료 확인
    if (CurrentStep >= AnimSteps)
    {
        // 타이머 정지
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
        
        // 최종 점수 설정
        CurrentTotalScore = TargetScore;
        
        // 애니메이션 상태 초기화
        bAnimating = false;
        
        // 애니메이션 중에 추가 점수가 있었다면 새 애니메이션 시작
        if (PendingScore != TargetScore)
        {
            AnimateScoreIncrease(PendingScore);
        }
    }
}

// 대기중인 점수 즉시 반영
void UTotalScoreWidget::ApplyPendingScore()
{
    // 애니메이션 종료 및 점수 바로 반영
    if (bAnimating)
    {
        GetWorld()->GetTimerManager().ClearTimer(ScoreAnimTimerHandle);
        bAnimating = false;
    }
    
    UpdateTotalScore(PendingScore);
}